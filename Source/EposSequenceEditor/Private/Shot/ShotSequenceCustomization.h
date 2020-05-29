// IDDN.FR.001.220036.001.S.P.2021.000.00000
// EPOS is subject to copyright © laws and is the legal and intellectual property of Praxinos,Inc - Year of publishing 2022

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "SequencerCustomizationManager.h"
#include "TransformData.h"
#include "Widgets/SWidget.h"

class UShotSequence;

/**
 * The sequencer customization for board sequences.
 */
class FShotSequenceCustomization
    : public ISequencerCustomization
{
public:
    ~FShotSequenceCustomization();

public:
    virtual void RegisterSequencerCustomization( FSequencerCustomizationBuilder& ioBuilder ) override;
    virtual void UnregisterSequencerCustomization() override;

private:
    void ExtendSequencerToolbar( FToolBarBuilder& ToolbarBuilder );
    TSharedRef<SWidget> MakeCameraMenu();
    TSharedRef<SWidget> MakePlaneMenu();
    TSharedRef<SWidget> MakeLighttableMenu();
    TSharedRef<SWidget> MakeDrawingMenu();
    TSharedRef<SWidget> MakeSettingsMenu();
    TSharedRef<SWidget> MakeTextureMenu();
    TSharedRef<SWidget> MakeHelpMenu();

    enum EMapping
    {
        kMap,
        kUnmap,
    };
    void ProcessCommands( TSharedPtr<FUICommandList> CommandList, EMapping iMap );

private:
    /**
     * Called before an actor or component transform changes
     *
     * @param Object The object whose transform is about to change
     */
    void OnPreTransformChanged( UObject& InObject );

    /**
     * Called when an actor or component transform changes
     *
     * @param Object The object whose transform has changed
     */
    void OnTransformChanged( UObject& InObject );

    /**
     * Called before an actor or component property changes.
     * Forward to OnPreTransformChanged if the property is transform related.
     *
     * @param InObject The object whose property is about to change
     * @param InPropertyChain the property that is about to change
     */
    void OnPrePropertyChanged( UObject* InObject, const class FEditPropertyChain& InPropertyChain );

    /**
     * Called before an actor or component property changes.
     * Forward to OnTransformChanged if the property is transform related.
     *
     * @param InObject The object whose property is about to change
     * @param InPropertyChangedEvent the property that changed
     */
    void OnPostPropertyChanged( UObject* InObject, struct FPropertyChangedEvent& InPropertyChangedEvent );

private:
    ESequencerDropResult OnSequencerAssetsDrop( const TArray<UObject*>& iAssets, const FAssetDragDropOp& iDragDropOp );
    ESequencerDropResult OnSequencerClassesDrop( const TArray<TWeakObjectPtr<UClass>>& iClasses, const FClassDragDropOp& iDragDropOp );
    ESequencerDropResult OnSequencerActorsDrop( const TArray<TWeakObjectPtr<AActor>>& iActors, const FActorDragDropOp& iDragDropOp );

private:
    ISequencer*     mSequencer;
    UShotSequence*  mShotSequence;

    /** Mapping of objects to their existing transform data (for comparing against new transform data) */
    TMap< TWeakObjectPtr<UObject>, FTransformData > mObjectToExistingTransform;
};
