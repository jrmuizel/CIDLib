//
// FILE NAME: CIDDocComp_Topics.Cpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 04/03/2015
//
// COPYRIGHT: $_CIDLib_CopyRight_$
//
//  $_CIDLib_CopyRight2_$
//
// DESCRIPTION:
//
//  The implementation of the class that represents a help topic (which is a
//  non-terminal node in our tree of topics and pages.)
//
// CAVEATS/GOTCHAS:
//
// LOG:
//
//  $_CIDLib_Log_$
//


// ----------------------------------------------------------------------------
//  Includes
// ----------------------------------------------------------------------------
#include    "CIDDocComp.hpp"



// ---------------------------------------------------------------------------
//   CLASS: TTopic
//  PREFIX: topic
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
//  TTopic: Constructors and Destructor
// ---------------------------------------------------------------------------

//
//  We have a big special case here that the topic name will be empty for the
//  root topic (and the parent path will be /).
//
TTopic::TTopic( const   TString&    strTitle
                , const TString&    strParTopicPath
                , const TString&    strTopicName
                , const TString&    strSrcPath) :

    m_pathSource(strSrcPath)
    , m_strParTopicPath(strParTopicPath)
    , m_strTitle(strTitle)
    , m_strTopicName(strTopicName)
{
    // Build up our own source directory
	if (!m_strTopicName.bIsEmpty())
        m_pathSource.AddLevel(m_strTopicName);

    // Build up our own topic path
    m_strTopicPath = strParTopicPath;
    if (!m_strTopicName.bIsEmpty())
    {
		if (m_strTopicPath.chLast() != kCIDLib::chForwardSlash)
			m_strTopicPath += kCIDLib::chForwardSlash;
        m_strTopicPath += strTopicName;
    }
}

TTopic::~TTopic()
{
}


// ---------------------------------------------------------------------------
//  TTopic: Public, non-virtual methods
// ---------------------------------------------------------------------------

// We parse our stuff, then recurse as required
tCIDLib::TBoolean TTopic::bParse(TXMLTreeParser& xtprsToUse)
{
    // Output the topic path we are parsing
    facCIDDocComp.strmOut() << L"\n   Parsing Topic: " << m_strTopicPath << kCIDLib::EndLn;

    // Build up the path to the topic file
    TPathStr pathTopicFile(m_pathSource);
    pathTopicFile.AddLevel(L"CIDTopic");
    pathTopicFile.AppendExt(L"xml");

    // Parse out all our info first
    tCIDLib::TBoolean bRes = xtprsToUse.bParseRootEntity
    (
        pathTopicFile
        , tCIDXML::EParseOpts::Validate
        , tCIDXML::EParseFlags::TagsNText
    );
    if (!bRes)
    {
        facCIDDocComp.ShowXMLParseErr(pathTopicFile);
        return kCIDLib::False;
    }

    // It parsed OK, so get the root node and let's start pulling our info out
    const TXMLTreeElement& xtnodeRoot = xtprsToUse.xtdocThis().xtnodeRoot();
    m_strTopicPage = xtnodeRoot.xtattrNamed(L"TopicPage").strValue();

    // Parse the pages and add those to our page list.
    tCIDLib::TCard4 c4At;
    {
        c4At = 0;
        const TXMLTreeElement& xtnodePages = xtnodeRoot.xtnodeFindElement
        (
            kCIDDocComp::strXML_PageMap, 0, c4At
        );

        tCIDLib::TBoolean bVirtual;
        TString strExtra1;
        TString strExtra2;
        TString strExt;
        TString strFileName;
        TString strExtTitle;
        TString strVirtual;
        const tCIDLib::TCard4 c4Count = xtnodePages.c4ChildCount();
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TXMLTreeNode& xtnodeCur = xtnodePages.xtnodeChildAt(c4Index);
            if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
                continue;

            const TXMLTreeElement& xtnodeElem = xtnodePages.xtnodeChildAtAsElement(c4Index);

            //
            //  These are required
            //
            if (!xtnodeElem.bAttrExists(kCIDDocComp::strXML_FileName, strFileName)
            ||  !xtnodeElem.bAttrExists(kCIDDocComp::strXML_FileExt, strExt))
            {
                facCIDDocComp.strmErr()
                    << L"The file name and extension are required"
                    << kCIDLib::EndLn;
                bRes = kCIDLib::False;

                // Can't continue processing this one
                continue;
            }

            //
            //  The external title can be empty, in which case the internal title from the
            //  page file will be used for this.
            //
            if (!xtnodeElem.bAttrExists(kCIDDocComp::strXML_Title, strExtTitle))
                strExtTitle.Clear();

            // Extra stuff is optional
            if (!xtnodeElem.bAttrExists(kCIDDocComp::strXML_Extra1, strExtra1))
                strExtra1.Clear();
            if (!xtnodeElem.bAttrExists(kCIDDocComp::strXML_Extra2, strExtra2))
                strExtra2.Clear();

            // And virtual is false if not set
            bVirtual = kCIDLib::False;
            if (xtnodeElem.bAttrExists(kCIDDocComp::strXML_Virtual, strVirtual))
            {
                if (!strVirtual.bToBoolean(bVirtual))
                {
                    facCIDDocComp.strmErr()
                        << L"Could not convert '" << kCIDDocComp::strXML_Virtual
                        << L"' value to a boolean" << kCIDLib::EndLn;

                    bRes = kCIDLib::False;
                }
            }

            //
            //  Based on extension type, let's gen up the right type of page and
            //  add it to our list.
            //
            if (strExt.bCompareI(kCIDDocComp::strExt_HelpPage))
            {
                THelpPage* ppgNew = new THelpPage
                (
                    strExtTitle, m_pathSource, m_strTopicPath, strFileName, strExt, bVirtual
                );
                m_colPages.objAdd(TPagePtr(ppgNew));
            }
             else
            {
                facCIDDocComp.strmErr()
                    << L"'" << strExt << L"' is not a known help file type"
                    << kCIDLib::EndLn;
                bRes = kCIDLib::False;
            }
        }
    }

    // Parse the sub-topics and add those to our sub-topic list.
    {
        c4At = 0;
        const TXMLTreeElement& xtnodeTopics = xtnodeRoot.xtnodeFindElement
        (
            kCIDDocComp::strXML_SubTopicMap, 0, c4At
        );
        const tCIDLib::TCard4 c4Count = xtnodeTopics.c4ChildCount();
        TString strTopicName;
        TString strTitle;
        for (tCIDLib::TCard4 c4Index = 0; c4Index < c4Count; c4Index++)
        {
            const TXMLTreeNode& xtnodeCur = xtnodeTopics.xtnodeChildAt(c4Index);
            if (xtnodeCur.eType() != tCIDXML::ENodeTypes::Element)
                continue;

            const TXMLTreeElement& xtnodeElem = xtnodeTopics.xtnodeChildAtAsElement(c4Index);
            if (!xtnodeElem.bAttrExists(kCIDDocComp::strXML_SubDir, strTopicName)
            ||  !xtnodeElem.bAttrExists(kCIDDocComp::strXML_Title, strTitle))
            {
                facCIDDocComp.strmErr()
                    << L"The topic sub-dir and title are required"
                    << kCIDLib::EndLn;
                bRes = kCIDLib::False;

                // Can't continue processing this one
                continue;
            }

            //
            //  Let's add this one to our list
            m_colSubTopics.objAdd
            (
                TTopicPtr(new TTopic(strTitle, m_strTopicPath, strTopicName, m_pathSource))
            );
        }
    }

    //
    //  Now let's go back and, for any pages not marked as virtual, let's let them
    //  parse their source files.
    //
    {
        TPageList::TNCCursor cursPages(&m_colPages);
        for (; cursPages; ++cursPages)
        {
            TPagePtr& cptrCur = *cursPages;
            if (!cptrCur->bVirtual())
            {
                if (!cptrCur->bParseFile(*this, xtprsToUse))
                {

                }
            }
        }
    }

    // And finally go back and recurse on any sub-topics
    {
        TSubTopicList::TNCCursor cursSubs(&m_colSubTopics);
        for (; cursSubs; ++cursSubs)
        {
            TTopicPtr& cptrCur = *cursSubs;
            cptrCur->bParse(xtprsToUse);
        }
    }

    return bRes;
}


// Find a page by its base file name, which has to be unique for us
TTopic::TPagePtr TTopic::cptrFindPage(const TString& strName) const
{
    TPageList::TCursor cursPages(&m_colPages);
    for (; cursPages; ++cursPages)
    {
        const TPagePtr& cptrCur = *cursPages;
        if (cptrCur->bIsThisName(strName))
            break;
    }

    if (cursPages)
        return *cursPages;

    // Just return an empty pointer
    return TPagePtr();
}


// Find a sub-topic by its base directory name, which has to be unique for us
TTopic::TTopicPtr TTopic::cptrFindTopic(const TString& strName) const
{
    TSubTopicList::TCursor cursTopics(&m_colSubTopics);
    for (; cursTopics; ++cursTopics)
    {
        const TTopicPtr& cptrCur = *cursTopics;
        if (cptrCur->bIsThisName(strName))
            break;
    }

    if (cursTopics)
        return *cursTopics;

    // Just return an empty pointer
    return TTopicPtr();
}


// If the's a defined topic page return that, else the first page in the list
TTopic::TPagePtr TTopic::cptrTopicPage() const
{
    if (m_strTopicPage.bIsEmpty())
        return m_colPages[0];

    TPagePtr cptrRet = cptrFindPage(m_strTopicPage);
    if (!cptrRet)
    {
        facCIDDocComp.strmErr()
            << L"No topic page was found for topic '" << m_strTopicPage << L"'"
            << kCIDLib::EndLn;
    }
    return cptrRet;
}


tCIDLib::TVoid
TTopic::GenerateOutput( const   TString&    strOutPath
                        , const TTopicPtr&  cptrParent
                        , const TTopicPtr&  cptrUs) const
{
    //
    //  Create our output path. It is the parent output path plus our topic name
    //  which is used to create a new sub-dir. If this is the root, then we don't
    //  do anything since its topic name is empty.
    //
    TPathStr pathOurPath(strOutPath);
    if (!m_strTopicName.bIsEmpty())
    {
        pathOurPath.AddLevel(m_strTopicName);

        if (!TFileSys::bIsDirectory(pathOurPath))
            TFileSys::MakeSubDirectory(m_strTopicName, strOutPath);
    }

    // Do our own index links page first, which will go into the left hand side
    {
        TPathStr pathTar = pathOurPath;
        pathTar.AddLevel(L"CIDTopicIndex");
        pathTar.AppendExt(L"html");

        TTextFileOutStream strmOutFl
        (
            pathTar
            , tCIDLib::ECreateActs::CreateAlways
            , tCIDLib::EFilePerms::Default
            , tCIDLib::EFileFlags::SequentialScan
            , tCIDLib::EAccessModes::Excl_Write
            , new TUTF8Converter
        );

        // Generate the topic index, which we break out separately
        GenerateTopicIndex(strmOutFl, cptrParent);

        // Make sure we get it flushed out before we drop it
        strmOutFl.Flush();
    }

    // Now iterate our pages at this level and spit those out
    {
        TPageList::TCursor cursPages(&m_colPages);
        for (; cursPages; ++cursPages)
        {
            const TPagePtr& cptrCur = *cursPages;
            cptrCur->GenerateOutput(pathOurPath);
        }
    }

    // And recurse on child topics
    {

        TSubTopicList::TCursor cursSubs(&m_colSubTopics);
        for (; cursSubs; ++cursSubs)
        {
            const TTopicPtr& cptrCur = *cursSubs;
            cptrCur->GenerateOutput(pathOurPath, cptrUs, cptrCur);
        }
    }
}


tCIDLib::TVoid TTopic::GenerateTopicLink(TTextOutStream& strmTar) const
{
    strmTar << L"<a onclick=\"javascript:loadTopic('"
            << m_strTopicPath
            << L"', '/"
            << m_strTopicPage
            << L"', true);\" href='javascript:void(0)'>"
            << m_strTitle
            << L"</a><br/>\n";
}


// ---------------------------------------------------------------------------
//  TTopic: Private, non-virtual methods
// ---------------------------------------------------------------------------

//
//  There's quite a bit of code involved to spit out the left side topic index links,
//  so we break it out.
//
tCIDLib::TVoid
TTopic::GenerateTopicIndex(         TTextOutStream& strmOutFl
                            , const TTopicPtr&      cptrParent) const
{
    // Do the doctype and any head stuff we want
    strmOutFl << L"<!DOCTYPE html>\n";

    // If we have a main topic page, then let's do it first
    if (!m_strTopicPage.bIsEmpty())
    {
        TPagePtr cptrMain = cptrFindPage(m_strTopicPage);
        if (cptrMain)
            cptrMain->GenerateLink(strmOutFl);

        // We want a break after this one if there's more than one
        if (m_colPages.c4ElemCount() > 1)
            strmOutFl << L"<br/>";
    }

    //
    //  Process the pages, outputting a link for each entry, skipping the main
    //  topic page this time.
    //
    {
        TPageList::TCursor cursPages = TPageList::TCursor(&m_colPages);
        for (; cursPages; ++cursPages)
        {
            const TPagePtr& cptrCur = *cursPages;

            // If not the topic page, then generate a link
            if (!cptrCur->bIsThisName(m_strTopicPage))
                cptrCur->GenerateLink(strmOutFl);
        }
    }

    //
    //  Now go through all of the sub-topics and do links that load the topic index for the
    //  topic into the left side, and the main topic page for the topic on the right.
    //
    //
    if (!m_colSubTopics.bIsEmpty())
    {
        // Space it down from the file list
        strmOutFl << L"<br/><br/>";

        TSubTopicList::TCursor cursTopics = TSubTopicList::TCursor(&m_colSubTopics);
        for (; cursTopics; ++cursTopics)
            (*cursTopics)->GenerateTopicLink(strmOutFl);
    }

    //
    //  If this is not the top level topic, then put out an up level link to go back
    //  to the previous topic. This avoids them having to do multiple back cmds to
    //  go back through all the stuff at this level before they can go back up to a
    //  previous level. It invokes a javacript function that knows it is getting a
    //  topic level link.
    //
    if (!m_strTopicName.bIsEmpty())
    {
        // Get the topic page for the parent topic. If not found it will show an error
        TPagePtr cptrTopic = cptrParent->cptrTopicPage();
        if (cptrTopic)
        {
            strmOutFl   << L"<br/><a onclick=\"javascript:loadTopic('"
                        << cptrParent->m_strTopicPath
                        << L"', '"
                        << cptrParent->m_strTopicPage
                        << L"', true);\" href='javascript:void(0)'>Up</a><p/>";
        }
    }
}


