// IDDN.FR.001.220036.001.S.P.2021.000.00000
// EPOS is subject to copyright © laws and is the legal and intellectual property of Praxinos,Inc - Year of publishing 2022

#include "StoryboardViewport/SStoryboardTransportRange.h"
#include "Rendering/DrawElements.h"
#include "SequencerKeyCollection.h"
#include "MovieSceneSequence.h"
#include "EditorStyleSet.h"
#include "Styles/EposSequenceEditorStyle.h"
#include "MovieScene.h"
#include "ISequencer.h"
#include "CommonMovieSceneTools.h"

#define LOCTEXT_NAMESPACE "SStoryboardTransportRange"

void SStoryboardTransportRange::Construct(const FArguments& InArgs)
{
    bDraggingTime = false;
}

void SStoryboardTransportRange::SetSequencer(TWeakPtr<ISequencer> InSequencer)
{
    WeakSequencer = InSequencer;
}

ISequencer* SStoryboardTransportRange::GetSequencer() const
{
    return WeakSequencer.Pin().Get();
}

FVector2D SStoryboardTransportRange::ComputeDesiredSize(float) const
{
    static const float MarkerHeight = 6.f;
    static const float TrackHeight = 8.f;
    return FVector2D(100.f, MarkerHeight + TrackHeight);
}

FReply SStoryboardTransportRange::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    bDraggingTime = true;
    SetTime(MyGeometry, MouseEvent);

    ISequencer* Sequencer = GetSequencer();
    if (Sequencer)
    {
        Sequencer->SetPlaybackStatus(EMovieScenePlayerStatus::Scrubbing);
    }

    return FReply::Handled().CaptureMouse(AsShared()).PreventThrottling();
}

FReply SStoryboardTransportRange::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    if (bDraggingTime)
    {
        SetTime(MyGeometry, MouseEvent);
    }

    return FReply::Handled();
}

FReply SStoryboardTransportRange::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    bDraggingTime = false;

    ISequencer* Sequencer = GetSequencer();
    if (Sequencer)
    {
        Sequencer->SetPlaybackStatus(EMovieScenePlayerStatus::Stepping);
    }

    return FReply::Handled().ReleaseMouseCapture();
}

void SStoryboardTransportRange::SetTime(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
    ISequencer* Sequencer = GetSequencer();
    if (Sequencer)
    {
        float Lerp = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition()).X / MyGeometry.GetLocalSize().X;
        Lerp = FMath::Clamp(Lerp, 0.f, 1.f);

        FMovieSceneEditorData& EditorData = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene()->GetEditorData();
        double NewTimeSeconds = EditorData.WorkStart + (EditorData.WorkEnd - EditorData.WorkStart) * Lerp;

        Sequencer->SetLocalTime(NewTimeSeconds * Sequencer->GetFocusedTickResolution(), ESnapTimeMode::STM_All);
    }
}

void SStoryboardTransportRange::OnMouseCaptureLost(const FCaptureLostEvent& CaptureLostEvent)
{
    bDraggingTime = false;
}

void SStoryboardTransportRange::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
    ISequencer* Sequencer = GetSequencer();
    if (Sequencer)
    {
        TRange<double> WorkingRange = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene()->GetEditorData().GetWorkingRange();

        // Anything within 3 pixel's worth of time is a duplicate as far as we're concerned
        FTimeToPixel TimeToPixelConverter(AllottedGeometry, WorkingRange, Sequencer->GetFocusedTickResolution());
        const float DuplicateThreshold = (TimeToPixelConverter.PixelToSeconds(3.f) - TimeToPixelConverter.PixelToSeconds(0.f));

        Sequencer->GetKeysFromSelection(ActiveKeyCollection, FMath::Max(DuplicateThreshold, SMALL_NUMBER));
    }
}

void SStoryboardTransportRange::DrawKeys(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId, bool bParentEnabled, const TArrayView<const FFrameNumber>& Keys, const TArrayView<const FLinearColor>& KeyColors, bool& bOutPlayMarkerOnKey) const
{
    ISequencer* Sequencer = GetSequencer();
    if (!Sequencer)
    {
        return;
    }

    const ESlateDrawEffect DrawEffects = bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;

    static const float TrackOffsetY = 6.f;
    const float TrackHeight = AllottedGeometry.GetLocalSize().Y - TrackOffsetY;

    FFrameRate           TickResolution  = Sequencer->GetFocusedTickResolution();
    TRange<double>       WorkingRange    = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene()->GetEditorData().GetWorkingRange();
    TRange<FFrameNumber> PlaybackRange   = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene()->GetPlaybackRange();

    const FFrameTime FramesPerPixel = (WorkingRange.Size<double>() / AllottedGeometry.GetLocalSize().X) * TickResolution;
    const float FullRange = WorkingRange.Size<float>();

    static const float BrushWidth = 7.f;
    static const float BrushHeight = 7.f;

    const float BrushOffsetY = TrackOffsetY + TrackHeight * .5f - BrushHeight * .5f;
    const FSlateBrush* KeyBrush = FEposSequenceEditorStyle::Get().GetBrush("CinematicViewportTransportRangeKey");
    const FQualifiedFrameTime CurrentTime = Sequencer->GetLocalTime();

    FLinearColor KeyColor = KeyColors.Num() ? KeyColors[0] : FAppStyle::Get().GetSlateColor("SelectionColor").GetColor(FWidgetStyle());

    int32 KeyIndex = 0;
    for (const FFrameNumber Time : Keys)
    {
        if (FMath::Abs(CurrentTime.Time - Time) < FramesPerPixel/2)
        {
            bOutPlayMarkerOnKey = true;
        }

        float Lerp = (Time/TickResolution - WorkingRange.GetLowerBoundValue()) / FullRange;

        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId+2,
            AllottedGeometry.ToPaintGeometry(
                FVector2D(BrushWidth, BrushHeight),
                FSlateLayoutTransform(FVector2D(AllottedGeometry.GetLocalSize().X*Lerp - BrushWidth*.5f, BrushOffsetY))
            ),
            KeyBrush,
            DrawEffects,
            KeyIndex < KeyColors.Num() ? KeyColors[KeyIndex] : KeyColor
        );

        ++KeyIndex;
    }
}

int32 SStoryboardTransportRange::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
    ISequencer* Sequencer = GetSequencer();

    const ESlateDrawEffect DrawEffects = bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
    if (!Sequencer)
    {
        return LayerId;
    }

    static const float TrackOffsetY = 6.f;
    const float TrackHeight = AllottedGeometry.GetLocalSize().Y - TrackOffsetY;

    FFrameRate           TickResolution  = Sequencer->GetFocusedTickResolution();
    TRange<double>       WorkingRange    = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene()->GetEditorData().GetWorkingRange();
    TRange<FFrameNumber> PlaybackRange   = Sequencer->GetFocusedMovieSceneSequence()->GetMovieScene()->GetPlaybackRange();

    // Anything within 3 pixel's worth of time is a duplicate as far as we're concerned
    FTimeToPixel TimeToPixelConverter(AllottedGeometry, WorkingRange, TickResolution);
    const float DuplicateThreshold = (TimeToPixelConverter.PixelToSeconds(3.f) - TimeToPixelConverter.PixelToSeconds(0.f));

    const FFrameTime FramesPerPixel = (WorkingRange.Size<double>() / AllottedGeometry.GetLocalSize().X) * TickResolution;

    FColor DarkGray(40, 40, 40);
    FColor MidGray(80, 80, 80);
    FColor LightGray(200, 200, 200);

    // Paint the left padding before the playback start
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        LayerId,
        AllottedGeometry.ToPaintGeometry( FVector2D(0.f, TrackOffsetY),  FVector2D(AllottedGeometry.GetLocalSize().X, TrackHeight)),
        FAppStyle::Get().GetBrush("WhiteBrush"),
        DrawEffects,
        FLinearColor(DarkGray)
    );

    const float FullRange = WorkingRange.Size<float>();

    const float PlaybackStartLerp   = (PlaybackRange.GetLowerBoundValue()/TickResolution - WorkingRange.GetLowerBoundValue()) / FullRange;
    const float PlaybackEndLerp     = (PlaybackRange.GetUpperBoundValue()/TickResolution - WorkingRange.GetLowerBoundValue()) / FullRange;

    // Draw the playback range
    FSlateDrawElement::MakeBox(
        OutDrawElements,
        ++LayerId,
        AllottedGeometry.ToPaintGeometry( FVector2D(AllottedGeometry.GetLocalSize().X*PlaybackStartLerp, TrackOffsetY),  FVector2D(AllottedGeometry.GetLocalSize().X*(PlaybackEndLerp - PlaybackStartLerp), TrackHeight)),
        FAppStyle::Get().GetBrush("WhiteBrush"),
        DrawEffects,
        FLinearColor(MidGray)
    );

    const FQualifiedFrameTime CurrentTime = Sequencer->GetLocalTime();
    const float ProgressLerp = (CurrentTime.AsSeconds() - WorkingRange.GetLowerBoundValue()) / FullRange;

    // Draw the playback progress
    if (ProgressLerp > PlaybackStartLerp)
    {
        const float ClampedProgressLerp = FMath::Clamp(ProgressLerp, PlaybackStartLerp, PlaybackEndLerp);
        FSlateDrawElement::MakeBox(
            OutDrawElements,
            ++LayerId,
            AllottedGeometry.ToPaintGeometry( FVector2D(AllottedGeometry.GetLocalSize().X*PlaybackStartLerp, TrackOffsetY), FVector2D(AllottedGeometry.GetLocalSize().X * (ClampedProgressLerp - PlaybackStartLerp), TrackHeight) ),
            FAppStyle::Get().GetBrush("WhiteBrush"),
            DrawEffects,
            FLinearColor(LightGray)
        );
    }

    bool bPlayMarkerOnKey = false;

    const FLinearColor KeyframeColor = FAppStyle::Get().GetSlateColor("SelectionColor").GetColor(FWidgetStyle());

    // Draw the current key collection tick marks
    if (ActiveKeyCollection.IsValid())
    {
        TRange<FFrameNumber> VisibleFrameRange(
            (WorkingRange.GetLowerBoundValue() * TickResolution).FloorToFrame(),
            (WorkingRange.GetUpperBoundValue() * TickResolution).CeilToFrame()
            );

        TArrayView<const FFrameNumber> Keys = ActiveKeyCollection->GetKeysInRange(VisibleFrameRange);

        DrawKeys(AllottedGeometry, OutDrawElements, LayerId, bParentEnabled, Keys, TArrayView<FLinearColor>(), bPlayMarkerOnKey);
    }

    // Draw the marked frames
    TArray<FMovieSceneMarkedFrame> MarkedFrames = Sequencer->GetMarkedFrames();
    if (MarkedFrames.Num())
    {
        int64 TotalMaxSeconds = static_cast<int64>(TNumericLimits<int32>::Max() / TickResolution.AsDecimal());

        FFrameNumber ThresholdFrames = (FMath::Max(DuplicateThreshold, SMALL_NUMBER) * TickResolution).FloorToFrame();
        if (ThresholdFrames.Value < -TotalMaxSeconds)
        {
            ThresholdFrames.Value = TotalMaxSeconds;
        }
        else if (ThresholdFrames.Value > TotalMaxSeconds)
        {
            ThresholdFrames.Value = TotalMaxSeconds;
        }

        TArray<FFrameNumber> GroupedTimes;
        TArray<FLinearColor> KeyColors;

        GroupedTimes.Reset(MarkedFrames.Num());
        KeyColors.Reset(MarkedFrames.Num());
        int32 Index = 0;
        while ( Index < MarkedFrames.Num() )
        {
            FFrameNumber PredicateTime = MarkedFrames[Index].FrameNumber;
            GroupedTimes.Add(PredicateTime);
            KeyColors.Add(MarkedFrames[Index].Color);
            KeyColors[KeyColors.Num()-1].A = 0.8f; // make the alpha consistent across all markers

            while (Index < MarkedFrames.Num() && FMath::Abs(MarkedFrames[Index].FrameNumber - PredicateTime) <= ThresholdFrames)
            {
                ++Index;
            }
        }
        GroupedTimes.Shrink();
        KeyColors.Shrink();

        DrawKeys(AllottedGeometry, OutDrawElements, LayerId, bParentEnabled, GroupedTimes, KeyColors, bPlayMarkerOnKey);
    }

    // Draw the play marker
    {
        static const float BrushWidth = 11.f, BrushHeight = 6.f;
        const float PositionX = AllottedGeometry.GetLocalSize().X * ProgressLerp;

        FSlateDrawElement::MakeBox(
            OutDrawElements,
            ++LayerId,
            AllottedGeometry.ToPaintGeometry(FVector2D(PositionX - FMath::CeilToFloat(BrushWidth/2), 0.f), FVector2D(BrushWidth, BrushHeight)),
            FEposSequenceEditorStyle::Get().GetBrush("CinematicViewportPlayMarker"),
            DrawEffects,
            bPlayMarkerOnKey ? KeyframeColor : LightGray
        );

        if (!bPlayMarkerOnKey)
        {
            TArray<FVector2D> LinePoints;
            LinePoints.Add(FVector2D(PositionX, BrushHeight));
            LinePoints.Add(FVector2D(PositionX, AllottedGeometry.GetLocalSize().Y));

            FSlateDrawElement::MakeLines(
                OutDrawElements,
                LayerId,
                AllottedGeometry.ToPaintGeometry(),
                LinePoints,
                DrawEffects,
                LightGray,
                false
            );
        }
    }

    // Draw the play bounds
    {
        static const float BrushWidth = 4.f;

        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId+1,
            AllottedGeometry.ToPaintGeometry(FVector2D(AllottedGeometry.GetLocalSize().X*PlaybackStartLerp, TrackOffsetY), FVector2D(BrushWidth, TrackHeight)),
            FEposSequenceEditorStyle::Get().GetBrush("CinematicViewportRangeStart"),
            DrawEffects,
            FColor(32, 128, 32) // 120, 75, 50 (HSV)
        );

        FSlateDrawElement::MakeBox(
            OutDrawElements,
            LayerId+1,
            AllottedGeometry.ToPaintGeometry(FVector2D(AllottedGeometry.GetLocalSize().X*PlaybackEndLerp - BrushWidth, TrackOffsetY), FVector2D(BrushWidth, TrackHeight)),
            FEposSequenceEditorStyle::Get().GetBrush("CinematicViewportRangeEnd"),
            DrawEffects,
            FColor(128, 32, 32) // 0, 75, 50 (HSV)
        );
    }

    return LayerId;
}

#undef LOCTEXT_NAMESPACE
