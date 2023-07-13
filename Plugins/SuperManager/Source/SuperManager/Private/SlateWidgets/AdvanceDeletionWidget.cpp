// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "SlateBasics.h"
#include "DebugHeader.h"
#include "SuperManager.h"

#define ListAll TEXT("List All Available Assets")
#define ListUnused TEXT("List Unused Assets")
#define ListSameName TEXT("List Assets With Same Name")

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true;

	StoredAssetsData = InArgs._AssetsDataToStore;
	DisplayedAssetsData = StoredAssetsData;

	CheckBoxesArray.Empty();
	AssetsDataToDeleteArray.Empty();
	ComboBoxSourceItems.Empty();

	ComboBoxSourceItems.Add(MakeShared<FString>(ListAll));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListUnused));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListSameName));

	FSlateFontInfo TitleTextFont = GetEmbossedTextFont();
	TitleTextFont.Size = 30;

	ChildSlot
		[
			//Main vertical box
			SNew(SVerticalBox)

			//First vertical slot for title text
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Advance Deletion")))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]

		//下拉列表框
		//SecondSlot for drop down to specify the listing condition and help text
	+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			//Combo Box Slot
			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				ConstructComboBox()
			]
		]
		
		//获取资产列表
		//Third slot for the asset list
		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)

			+SScrollBox::Slot()
			[
				ConstructAssetListView()
			]
		]
		
		//按钮点击
		//Fourth slot for 3 button
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			//Button1 slot
			+ SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructDeleteAllButton()
			]

			//Button2 slot
			+ SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructSelectAllButton()
			]

			//Button3 slot
			+ SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructDeselectAllButton()
			]

		]
	];
}

TSharedRef< SListView< TSharedPtr<FAssetData> > > SAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView = SNew(SListView< TSharedPtr<FAssetData> >)
		.ItemHeight(24.f)
		.ListItemsSource(&DisplayedAssetsData)
		.OnGenerateRow(this, &SAdvanceDeletionTab::OnGenerateRowForList)
		.OnMouseButtonClick(this, &SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked);

	return ConstructedAssetListView.ToSharedRef();
}

void SAdvanceDeletionTab::RefreshAssetListView()
{
	CheckBoxesArray.Empty();
	AssetsDataToDeleteArray.Empty();

	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

#pragma region ComboBoxForListingCondition

TSharedRef< SComboBox<TSharedPtr<FString>> > SAdvanceDeletionTab::ConstructComboBox()
{
	TSharedRef< SComboBox<TSharedPtr<FString>> > ConstructedComboBox =
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&ComboBoxSourceItems)
		.OnGenerateWidget(this, &SAdvanceDeletionTab::OnGenerateComboContent)
		.OnSelectionChanged(this, &SAdvanceDeletionTab::OnComboSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock, STextBlock)
			.Text(FText::FromString(TEXT("List Assets Option")))
		];

	return ConstructedComboBox;
}

TSharedRef<SWidget> SAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> ContructedComboText = SNew(STextBlock)
		.Text(FText::FromString(*SourceItem.Get()));

	return ContructedComboText;
}

void SAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	//DebugHeader::Print(*SelectedOption.Get(), FColor::Cyan);

	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));

	//加载 SuperManager 模块
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	//Pass data for our module to filter based on the selected option
	if (*SelectedOption.Get() == ListAll)
	{
		//List all stored asset data
		DisplayedAssetsData = StoredAssetsData;
		RefreshAssetListView();
	}
	else if (*SelectedOption.Get() == ListUnused)
	{
		//List all unused assets
		SuperManagerModule.ListUnusedAssetsForAssetList(StoredAssetsData, DisplayedAssetsData);
		RefreshAssetListView();
	}
	else if (*SelectedOption.Get() == ListSameName)
	{
		//List out all assets with same name
		SuperManagerModule.ListSameNameAssetsForAssetList(StoredAssetsData, DisplayedAssetsData);
		RefreshAssetListView();
	}
}

#pragma endregion

#pragma region RowWidgetForAssetListView

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow< TSharedPtr<FAssetData> >, OwnerTable);

	//获取资产类的名称
	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClass.ToString();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmbossedTextFont();
	AssetClassNameFont.Size = 10;

	FSlateFontInfo AssetNameFont = GetEmbossedTextFont();
	AssetNameFont.Size = 15;

	TSharedRef< STableRow< TSharedPtr<FAssetData> >> ListViewRowWidget =
	SNew(STableRow< TSharedPtr<FAssetData> >, OwnerTable).Padding(FMargin(5.f)) //行之间的间隔
	[
		SNew(SHorizontalBox)

		//First slot for check box
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.FillWidth(.05f)
		[
			ConstructCheckBox(AssetDataToDisplay)
		]

		//Second slot for displaying asset name
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		.FillWidth(.5f)//列之间的间距
		[
			ConstructTextForRowWidget(DisplayAssetClassName, AssetClassNameFont)
		]

		//Third slot for displaying  asset name
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			ConstructTextForRowWidget(DisplayAssetName, AssetNameFont)
		]

		//Fourth slot for a button
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		[
			ConstructButtonForFowWidget(AssetDataToDisplay)
		]

		//SNew(STextBlock)
		//.Text(FText::FromString(DisplayAssetName))
	];

	return ListViewRowWidget;
}

//鼠标点击行回调函数
void SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData)
{
	//DebugHeader::Print(ClickedData->AssetName.ToString() + TEXT(" is clicked"), FColor::Blue);
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	SuperManagerModule.SyncCBToClickedAssetForAssetList(ClickedData->ObjectPath.ToString());
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this, &SAdvanceDeletionTab::OnCheckBoxStateChanged, AssetDataToDisplay)
	.Visibility(EVisibility::Visible);

	CheckBoxesArray.Add(ConstructedCheckBox);

	return ConstructedCheckBox;
}

void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:

		//DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" is unchecked"), FColor::Red);
		if (AssetsDataToDeleteArray.Contains(AssetData))
		{
			AssetsDataToDeleteArray.Remove(AssetData);
		}
		break;

	case ECheckBoxState::Checked:

		//DebugHeader::Print(AssetData->AssetName.ToString() + TEXT(" is checked"), FColor::Green);
		AssetsDataToDeleteArray.AddUnique(AssetData);
		break;

	case ECheckBoxState::Undetermined:
		break;

	default:
		break;
	}

}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> ConstructTextBlock = SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(FontToUse)
	.ColorAndOpacity(FColor::White);

	return ConstructTextBlock;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForFowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton = SNew(SButton)
		.Text(FText::FromString(TEXT("Delete")))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeleteButtonClicked, AssetDataToDisplay);

	return ConstructedButton;
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	const bool bAssetDeleted = SuperManagerModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());

	if (bAssetDeleted)
	{
		//Updating the list source items
		if (StoredAssetsData.Contains(ClickedAssetData))
		{
			StoredAssetsData.Remove(ClickedAssetData);
		}

		if (DisplayedAssetsData.Contains(ClickedAssetData))
		{
			DisplayedAssetsData.Remove(ClickedAssetData);
		}

		//Refresh the list
		RefreshAssetListView();
	}

	//DebugHeader::Print(ClickedAssetData->AssetName.ToString() + TEXT(" is clicked"), FColor::Green);
	return FReply::Handled();
}

#pragma endregion

#pragma region TabButtons

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeleteAllButtonClicked);

	DeleteAllButton->SetContent(ConstructTextForTabButton(TEXT("Delete All")));

	return DeleteAllButton;
}

FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	//DebugHeader::Print(TEXT("Delete All Button Clicked"), FColor::Cyan);
	if (AssetsDataToDeleteArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok, TEXT("No asset currently selected"));
		return FReply::Handled();
	}

	TArray<FAssetData> AssetDataToDelete;
	//需要将数组中的共享指针转换成数组对象
	for (const TSharedPtr<FAssetData>& Data:AssetsDataToDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}

	//动态加载 SuperManager 模块
	FSuperManagerModule& SuperManagerModule =
		FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	const bool bAssetsDeleted = SuperManagerModule.DeleteMultipleAssetForAssetList(AssetDataToDelete);

	if (bAssetsDeleted)
	{
		//在视图列表数组中删除选中的资产数据，之后调用刷新视图列表函数
		for (const TSharedPtr<FAssetData>& DeletedDate:AssetsDataToDeleteArray)
		{
			if (StoredAssetsData.Contains(DeletedDate))
			{
				StoredAssetsData.Remove(DeletedDate);
			}

			if (DisplayedAssetsData.Contains(DeletedDate))
			{
				DisplayedAssetsData.Remove(DeletedDate);
			}
		}

		RefreshAssetListView();
	}

	return FReply::Handled();
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnSelectAllButtonClicked);

	SelectAllButton->SetContent(ConstructTextForTabButton(TEXT("Selete All")));

	return SelectAllButton;
}

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
{
	if (CheckBoxesArray.Num() == 0) return FReply::Handled();
	
	for (const TSharedRef<SCheckBox>& CheckBox:CheckBoxesArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}

	return FReply::Handled();
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton = SNew(SButton)
		.ContentPadding(FMargin(5.f))
		.OnClicked(this, &SAdvanceDeletionTab::OnDeselectAllButtonClicked);

	DeselectAllButton->SetContent(ConstructTextForTabButton(TEXT("Deselete All")));

	return DeselectAllButton;
}

FReply SAdvanceDeletionTab::OnDeselectAllButtonClicked()
{
	//DebugHeader::Print(TEXT("Deselete All Button Clicked"), FColor::Cyan);
	if (CheckBoxesArray.Num() == 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox>& CheckBox:CheckBoxesArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}

	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForTabButton(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmbossedTextFont();
	ButtonTextFont.Size = 15;

	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
		.Text(FText::FromString(TextContent))
		.Font(ButtonTextFont)
		.Justification(ETextJustify::Center);

	return ConstructedTextBlock;
}

#pragma endregion