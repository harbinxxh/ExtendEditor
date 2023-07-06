// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "SlateWidgets/AdvanceDeletionWidget.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	InitCBMenuExtention();
	RegisterAdvanceDeletionTab();
}

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#pragma region ContentBrowserMenuExtention

void FSuperManagerModule::InitCBMenuExtention()
{
	FContentBrowserModule& ContentBrowserModule =
	FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	//右键单击文件夹时，请求弹出的菜单路径
	//Called to request the menu when right clicking on a path
	//Get hold of all the menu extenders
	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = 
	ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	// 代理第一种方式
	//FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	//CustomCBMenuDelegate.BindRaw(this, &FSuperManagerModule::CustomCBMenuExtender);
	//ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);

	//  代理第二种方式
	//We add custom delegate to all the existing delegates
	ContentBrowserModuleMenuExtenders.Add(FContentBrowserMenuExtender_SelectedPaths::
		CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"), //Extention hook, position to insert
			EExtensionHook::After, //Inserting before or after
			TSharedPtr<FUICommandList>(), //Custom hot keys
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)); //Second binding, will define details for this menu entry
		
		//把获取到的文件夹信息保存到 FolderPathsSelected 成员变量中
		FolderPathsSelected = SelectedPaths;
	}

	return MenuExtender;
}

//Define deatils for the custom menu entry
void FSuperManagerModule::AddCBMenuEntry(class FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Unused Assets")), //Title text for menu entry
		FText::FromString(TEXT("Safely delete all unused assets under folder")), //Tooltip text
		FSlateIcon(), //Custom icon
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtionClicked) //The actual function to excute
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely delete all empty folder")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked)
	);

	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("Advance Deletion")),
		FText::FromString(TEXT("List assets by specific condition in a tab for deleting")),
		FSlateIcon(),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked)
	);
}

void FSuperManagerModule::OnDeleteUnusedAssetButtionClicked()
{
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("You can only do this to one folder"));
		return;
	}
	//DebugHeader::Print(TEXT("Currently selected folder: ") + FolderPathsSelected[0], FColor::Green);

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	//Whether there are assets under the folder
	if (AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset found under selected folder"), false);
	}

	EAppReturnType::Type ConfirmResult =
		DebugHeader::ShowMsgDialog(EAppMsgType::YesNo, TEXT("A total of ") + FString::FromInt(AssetsPathNames.Num()) + 
			TEXT(" assets need to be checked.\nWould you like to proccceed?"), false);

	if (ConfirmResult == EAppReturnType::No) return;

	FixUpRedirectors();

	TArray<FAssetData> UnusedAssetsDataArray;

	for (const FString& AssetPathName:AssetsPathNames)
	{
		//Don't touch root folder
		if ( AssetPathName.Contains(TEXT("Collections")) ||
			AssetPathName.Contains(TEXT("Developers")) ||
			AssetPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		TArray<FString> AssetRefrencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		if (AssetRefrencers.Num() == 0)
		{
			const FAssetData UnusedData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDataArray.Add(UnusedData);
		}
	}

	if (UnusedAssetsDataArray.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetsDataArray);
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No unused asset found under selected folder"), false);
	}
}

void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	FixUpRedirectors();

	TArray<FString> FolderPathsArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0], true, true);
	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	for (const FString& FolderPath:FolderPathsArray)
	{
		if (FolderPath.Contains(TEXT("Collections")) ||
			FolderPath.Contains(TEXT("Developers")) ||
			FolderPath.Contains(TEXT("__ExternalActors__")) ||
			FolderPath.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);
		}
	}

	if (EmptyFoldersPathsArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No empty folder found under selected folder"), false);
		return;
	}

	//Delete Empty Folder
	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel, 
		TEXT("Empty folders found in:\n") + EmptyFolderPathsNames + TEXT("\nWould you like to delete all?"), false);

	if (ConfirmResult == EAppReturnType::Cancel) return;

	for (const FString& EmptyFolderPath:EmptyFoldersPathsArray)
	{
		UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath) ?
			++Counter : DebugHeader::Print(TEXT("Failed to delete " + EmptyFolderPath), FColor::Red);
	}

	if (Counter > 0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Successfully delete ") + FString::FromInt(Counter) + TEXT(" folder"));
	}
}

void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

void FSuperManagerModule::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	FAssetRegistryModule& AssetRegistryModule =
		FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursiveClasses = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	FAssetToolsModule& AssetToolsModule =
		FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}

#pragma endregion

#pragma region CustomEditorTab

void FSuperManagerModule::RegisterAdvanceDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this, &FSuperManagerModule::OnSpawnAdvanceDeletionTab)
	).SetDisplayName(FText::FromString(TEXT("Advance Deletion")));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return
		SNew(SDockTab).TabRole(ETabRole::NomadTab)
		[
			SNew(SAdvanceDeletionTab)
			.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
		];
}

TArray< TSharedPtr<FAssetData> > FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray< TSharedPtr<FAssetData> > AvaiableAssetData;

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	for (const FString& AssetsPathName : AssetsPathNames)
	{
		if (AssetsPathName.Contains(TEXT("Collections")) ||
			AssetsPathName.Contains(TEXT("Developers")) ||
			AssetsPathName.Contains(TEXT("__ExternalActors__")) ||
			AssetsPathName.Contains(TEXT("__ExternalObjects__")))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetsPathName)) continue;

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetsPathName);

		AvaiableAssetData.Add(MakeShared<FAssetData>(Data));
	}

	return AvaiableAssetData;
}

#pragma endregion


#pragma region ProcessDataForAdvanceDeletionTab

bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);

	if (ObjectTools::DeleteAssets(AssetDataForDeletion) > 0)
	{
		return true;
	}

	return false;
}

bool FSuperManagerModule::DeleteMultipleAssetForAssetList(const TArray<FAssetData>& AssetsToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsToDelete))
	{
		return true;
	}

	return false;
}

void FSuperManagerModule::ListUnusedAssetsForAssetList(const TArray< TSharedPtr<FAssetData> >& AssetsDataToFilter, TArray< TSharedPtr<FAssetData> >& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();

	for (const TSharedPtr<FAssetData>& DataSharedPtr:AssetsDataToFilter)
	{
		//查找未使用的资产
		TArray<FString> AssetReferencers =
			UEditorAssetLibrary::FindPackageReferencersForAsset(DataSharedPtr->ObjectPath.ToString());

		if (AssetReferencers.Num() == 0)
		{
			OutUnusedAssetsData.Add(DataSharedPtr);
		}
	}
}

#pragma endregion

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)