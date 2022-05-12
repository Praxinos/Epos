// IDDN.FR.001.220036.001.S.P.2021.000.00000
// EPOS is subject to copyright © laws and is the legal and intellectual property of Praxinos,Inc - Year of publishing 2022

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class UDefaultNamingFormatterBoard;
class UDefaultNamingFormatterShot;

class FEposNamingConventionModule
    : public IModuleInterface
    , public FGCObject
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    /** FGCObject interface */
    virtual void AddReferencedObjects( FReferenceCollector& Collector ) override;
    virtual FString GetReferencerName() const override;

private:
    void RegisterPropertyCustomizations();
    void UnregisterPropertyCustomizations();

    void RegisterNamingFormatter();
    void UnregisterNamingFormatter();

private:
    UDefaultNamingFormatterBoard* mNamingFormatterBoard;
    UDefaultNamingFormatterShot* mNamingFormatterShot;
};
