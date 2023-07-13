// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

#pragma region ContentBrowserMenuExtention

	void InitCBMenuExtention();

	TArray<FString> FolderPathsSelected;

	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);

	void OnDeleteUnusedAssetButtionClicked();
	void OnDeleteEmptyFoldersButtonClicked();
	void OnAdvanceDeletionButtonClicked();

	void FixUpRedirectors();

#pragma endregion

public:

#pragma region CustomEditorTab

	void RegisterAdvanceDeletionTab();

	TSharedRef<SDockTab> OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs);

	TArray< TSharedPtr<FAssetData> > GetAllAssetDataUnderSelectedFolder();

#pragma endregion

#pragma region ProcessDataForAdvanceDeletionTab
	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);
	bool DeleteMultipleAssetForAssetList(const TArray<FAssetData>& AssetsToDelete);
	void ListUnusedAssetsForAssetList(const TArray< TSharedPtr<FAssetData> >& AssetsDataToFilter, TArray< TSharedPtr<FAssetData> >& OutUnusedAssetsData);
	void ListSameNameAssetsForAssetList(const TArray< TSharedPtr<FAssetData> >& AssetsDataToFilter, TArray< TSharedPtr<FAssetData> >& OutSameNameAssetsData);
	void SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync);//将内容浏览器同步到给定的资源

#pragma endregion
};
