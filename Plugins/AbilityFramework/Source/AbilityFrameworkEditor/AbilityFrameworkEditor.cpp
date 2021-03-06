// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "AbilityFrameworkEditor.h"
#include "GAAttributePin.h"
#include "Attributes/GAAttributeGlobals.h"
#include "Effects/GAEffectCue.h"
#include "GAAttributePanelGraphPinFactory.h"
#include "GAAttributeDetailCustomization.h"

#include "GAEffectDetails.h"
//#include "GAEffectSpecStructCustomization.h"
#include "GAEffectPropertyStructCustomization.h"
#include "AFAbilityActionSpecDetails.h"
#include "AFAbilityPeriodSpecDetails.h"
#include "AFAbilityCooldownSpecDetails.h"
#include "AFAbilityInfiniteDurationSpecDetails.h"
#include "AFAbilityInfinitePeriodSpecDetails.h"

#include "EffectEditor/AssetTypeActions_GAEffectBlueprint.h"
#include "AbilityEditor/AssetTypeActions_GAAbilityBlueprint.h"
#include "EffectCueEditor/AssetTypeActions_GAEffectCueBlueprint.h"
#include "EffectCueEditor/AFEffectCueDetails.h"



#include "AFCueManager.h"

TSet<UClass*> FAbilityFrameworkEditor::EffectClasses = TSet<UClass*>();
/** Shared class type that ensures safe binding to RegisterBlueprintEditorTab through an SP binding without interfering with module ownership semantics */
FEffectCueequenceEditorTabBinding::FEffectCueequenceEditorTabBinding()
{
	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	BlueprintEditorTabSpawnerHandle = BlueprintEditorModule.OnRegisterTabsForEditor().AddRaw(this, &FEffectCueequenceEditorTabBinding::RegisterBlueprintEditorTab);
	BlueprintEditorLayoutExtensionHandle = BlueprintEditorModule.OnRegisterLayoutExtensions().AddRaw(this, &FEffectCueequenceEditorTabBinding::RegisterBlueprintEditorLayout);
}

void FEffectCueequenceEditorTabBinding::RegisterBlueprintEditorLayout(FLayoutExtender& Extender)
{
	Extender.ExtendLayout(FBlueprintEditorTabs::CompilerResultsID, ELayoutExtensionPosition::Before, FTabManager::FTab(FName("EmbeddedEffectCueSequenceID"), ETabState::ClosedTab));
}

void FEffectCueequenceEditorTabBinding::RegisterBlueprintEditorTab(FWorkflowAllowedTabSet& TabFactories, FName InModeName, TSharedPtr<FBlueprintEditor> BlueprintEditor)
{
	TabFactories.RegisterFactory(MakeShared<FEffectCueSequenceEditorSummoner>(BlueprintEditor));
}

FEffectCueequenceEditorTabBinding::~FEffectCueequenceEditorTabBinding()
{
	FBlueprintEditorModule* BlueprintEditorModule = FModuleManager::GetModulePtr<FBlueprintEditorModule>("Kismet");
	if (BlueprintEditorModule)
	{
		BlueprintEditorModule->OnRegisterTabsForEditor().Remove(BlueprintEditorTabSpawnerHandle);
		BlueprintEditorModule->OnRegisterLayoutExtensions().Remove(BlueprintEditorLayoutExtensionHandle);
	}
}


void FAbilityFrameworkEditor::RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
	AssetTools.RegisterAssetTypeActions(Action);
	CreatedAssetTypeActions.Add(Action);
}
void FAbilityFrameworkEditor::OnInitializeSequence(UGAEffectCueSequence* Sequence)
{
	auto* ProjectSettings = GetDefault<UMovieSceneToolsProjectSettings>();
	AGAEffectCue* Cue = Cast<AGAEffectCue>(Sequence->GetOuter());
	if (Cue)
	{
		Sequence->GetMovieScene()->SetPlaybackRange(Cue->StartTime, Cue->EndTime);
	}
	else
	{
		Sequence->GetMovieScene()->SetPlaybackRange(ProjectSettings->DefaultStartTime, ProjectSettings->DefaultStartTime + ProjectSettings->DefaultDuration);
	}
}
	/** IModuleInterface implementation */
void FAbilityFrameworkEditor::StartupModule()
{
	if(GEditor)
		BlueprintEditorTabBinding = MakeShared<FEffectCueequenceEditorTabBinding>();
	
	

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout("GAAttribute", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGAAttributeDetailCustomization::MakeInstance));
	PropertyModule.RegisterCustomPropertyTypeLayout("GAEffectProperty", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FGAEffectPropertyStructCustomization::MakeInstance));

	TSharedPtr<FGAAttributePanelGraphPinFactory> GAAttributePanelGraphPinFactory = MakeShareable(new FGAAttributePanelGraphPinFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(GAAttributePanelGraphPinFactory);

	PropertyModule.RegisterCustomClassLayout("AFEffectSpec", FOnGetDetailCustomizationInstance::CreateStatic(&FGAEffectDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout("AFAbilityActivationSpec", FOnGetDetailCustomizationInstance::CreateStatic(&FAFAbilityActivationSpecDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout("AFAbilityPeriodSpec", FOnGetDetailCustomizationInstance::CreateStatic(&FAFAbilityPeriodSpecDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout("AFAbilityCooldownSpec", FOnGetDetailCustomizationInstance::CreateStatic(&FAFAbilityCooldownSpecDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout("AFAbilityPeriodicInfiniteSpec", FOnGetDetailCustomizationInstance::CreateStatic(&FAFAbilityInfinitePeriodSpecDetails::MakeInstance));
	PropertyModule.RegisterCustomClassLayout("AFAbilityInfiniteDurationSpec", FOnGetDetailCustomizationInstance::CreateStatic(&FAFAbilityInfiniteDurationSpecDetails::MakeInstance));

	PropertyModule.RegisterCustomClassLayout("GAEffectCue", FOnGetDetailCustomizationInstance::CreateStatic(&FAFEffectCueDetails::MakeInstance));

	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	TSharedRef<IAssetTypeActions> GABAction = MakeShareable(new FAssetTypeActions_GAEffectBlueprint());
	RegisterAssetTypeAction(AssetTools, GABAction);
	TSharedRef<IAssetTypeActions> GAAbilityAction = MakeShareable(new FAssetTypeActions_GAAbilityBlueprint());
	RegisterAssetTypeAction(AssetTools, GAAbilityAction);
	TSharedRef<IAssetTypeActions> GAEffectCueAction = MakeShareable(new FAssetTypeActions_GAEffectCueBlueprint());
	RegisterAssetTypeAction(AssetTools, GAEffectCueAction);

	//BlueprintEditorTabBinding = MakeShared<FEffectCueSequenceEditorTabBinding>();
	OnInitializeSequenceHandle = UGAEffectCueSequence::OnInitializeSequence().AddStatic(FAbilityFrameworkEditor::OnInitializeSequence);
}
void FAbilityFrameworkEditor::ShutdownModule()
{
	BlueprintEditorTabBinding = nullptr;
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomClassLayout("AFEffectSpec");
	PropertyModule.UnregisterCustomClassLayout("AFAbilityActivationSpec");
	PropertyModule.UnregisterCustomClassLayout("AFAbilityPeriodSpec");
	PropertyModule.UnregisterCustomClassLayout("AFAbilityCooldownSpec");
	PropertyModule.UnregisterCustomClassLayout("AFAbilityPeriodicInfiniteSpec");
	PropertyModule.UnregisterCustomClassLayout("AFAbilityInfiniteDurationSpec");

	PropertyModule.UnregisterCustomPropertyTypeLayout("GAAttribute");
	PropertyModule.UnregisterCustomPropertyTypeLayout("GAEffectProperty");

	UGAEffectCueSequence::OnInitializeSequence().Remove(OnInitializeSequenceHandle);
	BlueprintEditorTabBinding = nullptr;
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetToolsModule = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
		for (auto& AssetTypeAction : CreatedAssetTypeActions)
		{
			if (AssetTypeAction.IsValid())
			{
				AssetToolsModule.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
			}
		}
	}
}

IMPLEMENT_GAME_MODULE(FAbilityFrameworkEditor, AbilityFrameworkEditor);


//void FGameAttributesEditor::StartupModule()
//{
//
//}
//
//
//void FGameAttributesEditor::ShutdownModule()
//{
//
//}



