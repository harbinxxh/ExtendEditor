// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class SAdvanceDeletionTab : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab) {}

	SLATE_ARGUMENT(TArray< TSharedPtr<FAssetData> >, AssetsDataToStore)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TArray< TSharedPtr<FAssetData> > StoredAssetsData;
	TArray< TSharedPtr<FAssetData> > DisplayedAssetsData;//保存符合过滤条件的资产数据
	TArray< TSharedRef<SCheckBox> > CheckBoxesArray;//用于保存复选框对象
	TArray< TSharedPtr<FAssetData> > AssetsDataToDeleteArray;

	TSharedRef< SListView< TSharedPtr<FAssetData> > > ConstructAssetListView(); // 构造列表视图的函数
	TSharedPtr< SListView< TSharedPtr<FAssetData> > > ConstructedAssetListView; // 保存列表视图对象的引用
	void RefreshAssetListView();

#pragma region ComboBoxForListingCondition

	TSharedRef< SComboBox<TSharedPtr<FString>> > ConstructComboBox();

	TArray< TSharedPtr<FString> > ComboBoxSourceItems;//组合框数据源项条件选项

	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);//生成条件选项控件

	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo);//下拉框选择函数

	TSharedPtr<STextBlock> ComboDisplayTextBlock;

#pragma endregion

#pragma region RowWidgetForAssetListView

	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	void OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData);

	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse);

	TSharedRef<SButton> ConstructButtonForFowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);

#pragma endregion

#pragma region TabButtons

	TSharedRef<SButton> ConstructDeleteAllButton();
	FReply OnDeleteAllButtonClicked();

	TSharedRef<SButton> ConstructSelectAllButton();
	FReply OnSelectAllButtonClicked();

	TSharedRef<SButton> ConstructDeselectAllButton();
	FReply OnDeselectAllButtonClicked();

	TSharedRef<STextBlock> ConstructTextForTabButton(const FString& TextContent);

#pragma endregion

	FSlateFontInfo GetEmbossedTextFont() const { return FCoreStyle::Get().GetFontStyle(FName("EmbossedText")); }
};
