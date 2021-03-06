//
// FILE NAME: TestCIDLib2_DirChange.cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 10/22/2018
//
// COPYRIGHT: Charmed Quark Systems, Ltd @ 2019
//
//  This software is copyrighted by 'Charmed Quark Systems, Ltd' and
//  the author (Dean Roddey.) It is licensed under the MIT Open Source
//  license:
//
//  https://opensource.org/licenses/MIT
//
// DESCRIPTION:
//
//  This file contains tests related to the directory changes.
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ---------------------------------------------------------------------------
//  Include underlying headers
// ---------------------------------------------------------------------------
#include    "TestCIDLib2.hpp"


// ---------------------------------------------------------------------------
//  Magic macros
// ---------------------------------------------------------------------------
RTTIDecls(TTest_DirChange1, TTestFWTest)



// ---------------------------------------------------------------------------
//  CLASS: TTest_DirChange1
// PREFIX: tfwt
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTest_DirChange1: Constructor and Destructor
// ---------------------------------------------------------------------------
TTest_DirChange1::TTest_DirChange1() :

    TTestFWTest
    (
        L"Dir Changes 1", L"Basic tests of directory change monitoring", 4
    )
{
}

TTest_DirChange1::~TTest_DirChange1()
{
}


// ---------------------------------------------------------------------------
//  TTest_DirChange1: Public, inherited methods
// ---------------------------------------------------------------------------
tTestFWLib::ETestRes
TTest_DirChange1::eRunTest( TTextStringOutStream&   strmOut
                            , tCIDLib::TBoolean&    bWarning)
{
    tTestFWLib::ETestRes eRes = tTestFWLib::ETestRes::Success;

    //
    //  We have a DirChanges directory under our test source directory that we can
    //  use. First we want to clear its contents.
    //
    TPathStr pathTest;
    TFileSys::QueryCurrentDir(pathTest);
    pathTest.AddLevels(L"TestCIDLib2", L"DirChanges");
    TFileSys::CleanPath(pathTest, tCIDLib::ETreeDelModes::Clean);

    // Now set up monitoring on that path
    TDirChangeMon dchmTest;
    dchmTest.Start(pathTest, tCIDLib::EDirChFilters::AllWrite, kCIDLib::True);

    // Do an initial read which shouldn't see anything
    tCIDLib::TDirChanges colChanges;
    if (dchmTest.bReadChanges(colChanges))
    {
        strmOut << TFWCurLn << L"Got initial changes before doing anything\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }

    //
    //  Create a file. Set write-through, to make sure that there's no caching and
    //  we see changes.
    //
    TPathStr pathSrcFile(pathTest);
    pathSrcFile.AddLevel(L"Test1");
    pathSrcFile.AppendExt(L"TestFile");
    {
        TBinaryFile bflTest(pathSrcFile);
        bflTest.Open
        (
            tCIDLib::EAccessModes::Excl_ReadWrite
            , tCIDLib::ECreateActs::CreateIfNew
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::WriteThrough
        );

        TThread::Sleep(350);
        if (!dchmTest.bReadChanges(colChanges))
        {
            strmOut << TFWCurLn << L"No events generated by file creation\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
         else
        {
            // We should have gotten a single creation event
            if (colChanges.c4ElemCount() == 1)
            {
                const TDirChangeInfo& dchiCur = colChanges[0];
                if (dchiCur.m_eChange != tCIDLib::EDirChanges::Added)
                {
                    strmOut << TFWCurLn << L"Expected an added event but got "
                            << tCIDLib::TInt4(dchiCur.m_eChange) << L"\n\n";
                    eRes = tTestFWLib::ETestRes::Failed;
                }

                if (dchiCur.m_strName != L"Test1.TestFile")
                {
                    strmOut << TFWCurLn << L"Reported file name was "
                            << dchiCur.m_strName << L"\n\n";
                    eRes = tTestFWLib::ETestRes::Failed;
                }
            }
             else
            {
                strmOut << TFWCurLn << L"Got " << colChanges.c4ElemCount()
                        << L" events for file creation\n\n";
                eRes = tTestFWLib::ETestRes::Failed;
            }
        }

        // Now write to the it and we should get a modified event
        tCIDLib::TCard4 c4Val = kCIDLib::c4MaxCard;
        bflTest.c4WriteBuffer(&c4Val, sizeof(c4Val));
        bflTest.Flush();

        TThread::Sleep(350);
        if (!dchmTest.bReadChanges(colChanges))
        {
            strmOut << TFWCurLn << L"No events generated by file write\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
         else
        {
            // We should have gotten a single modified event
            if (colChanges.c4ElemCount() == 1)
            {
                const TDirChangeInfo& dchiCur = colChanges[0];
                if (dchiCur.m_eChange != tCIDLib::EDirChanges::Modified)
                {
                    strmOut << TFWCurLn << L"Expected a modified event but got "
                            << tCIDLib::TInt4(dchiCur.m_eChange) << L"\n\n";
                    eRes = tTestFWLib::ETestRes::Failed;
                }

                if (dchiCur.m_strName != L"Test1.TestFile")
                {
                    strmOut << TFWCurLn << L"Reported file name was "
                            << dchiCur.m_strName << L"\n\n";
                    eRes = tTestFWLib::ETestRes::Failed;
                }
            }
             else
            {
                strmOut << TFWCurLn << L"Got " << colChanges.c4ElemCount()
                        << L" events for file write\n\n";
                eRes = tTestFWLib::ETestRes::Failed;
            }
        }
    }

    //
    //  Now let's try a rename on the file we created above. Our wrapper should
    //  condense this to a single operation.
    //
    TPathStr pathNew;
    TFileSys::QueryCurrentDir(pathNew);
    pathNew.AddLevels(L"TestCIDLib2", L"DirChanges", L"Test1");
    pathNew.AppendExt(L"Renamed");
    TFileSys::Rename(pathSrcFile, pathNew);

    TThread::Sleep(350);
    if (!dchmTest.bReadChanges(colChanges))
    {
        strmOut << TFWCurLn << L"No events generated by file rename\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }
     else
    {
        if (colChanges.c4ElemCount() == 1)
        {
            const TDirChangeInfo& dchiCur = colChanges[0];
            if (dchiCur.m_eChange != tCIDLib::EDirChanges::Renamed)
            {
                strmOut << TFWCurLn << L"Expected a renamed event but got "
                        << tCIDLib::TInt4(dchiCur.m_eChange) << L"\n\n";
                eRes = tTestFWLib::ETestRes::Failed;
            }

            if (dchiCur.m_strName != L"Test1.TestFile")
            {
                strmOut << TFWCurLn << L"Reported old file name was "
                        << dchiCur.m_strName << L"\n\n";
                eRes = tTestFWLib::ETestRes::Failed;
            }

            if (dchiCur.m_strNew != L"Test1.Renamed")
            {
                strmOut << TFWCurLn << L"Reported new file name was "
                        << dchiCur.m_strName << L"\n\n";
                eRes = tTestFWLib::ETestRes::Failed;
            }
        }
         else
        {
            strmOut << TFWCurLn << L"Got " << colChanges.c4ElemCount()
                    << L" events for file rename\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // And let's do a delete
    TFileSys::DeleteFile(pathNew);

    TThread::Sleep(350);
    if (!dchmTest.bReadChanges(colChanges))
    {
        strmOut << TFWCurLn << L"No events generated by file delete\n\n";
        eRes = tTestFWLib::ETestRes::Failed;
    }
     else
    {
        if (colChanges.c4ElemCount() == 1)
        {
            const TDirChangeInfo& dchiCur = colChanges[0];
            if (dchiCur.m_eChange != tCIDLib::EDirChanges::Removed)
            {
                strmOut << TFWCurLn << L"Expected a removed event but got "
                        << tCIDLib::TInt4(dchiCur.m_eChange) << L"\n\n";
                eRes = tTestFWLib::ETestRes::Failed;
            }

            if (dchiCur.m_strName != L"Test1.Renamed")
            {
                strmOut << TFWCurLn << L"Reported deleted file name was "
                        << dchiCur.m_strName << L"\n\n";
                eRes = tTestFWLib::ETestRes::Failed;
            }
        }
         else
        {
            strmOut << TFWCurLn << L"Got " << colChanges.c4ElemCount()
                    << L" events for file delete\n\n";
            eRes = tTestFWLib::ETestRes::Failed;
        }
    }

    // Stop the monitoring
    dchmTest.Stop();

    return eRes;
}

