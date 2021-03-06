//
// FILE NAME: CIDXML_CMNodes_.hpp
//
// AUTHOR: Dean Roddey
//
// CREATED: 11/25/1999
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
//  This is the header file for the CIDXML_CMNodes.Cpp file, which implements
//  the TXMLCMNode class, and a set of simple derivatives. This is a private
//  header that is only needed by the TXMLChildCM class needs to see.
//
//  We have derived node classes that represent unary operations (*,+,?) and
//  binary operators (| and ,) and leaf nodes (which hold actual element ids.)
//
// CAVEATS/GOTCHAS:
//
//  1)  None of these are exported or seen in any public header. They are not
//      derived from TObject since no one ever sees them, so we can avoid the
//      overhead.
//
//  2)  Though much of these classes are small and could be inline, they are
//      used purely in a polymorphic way so its not worth the mess. Just a
//      couple non-virtual high runners in the base class are inlined.
//
// LOG:
//
//  $_CIDLib_Log_$
//
#pragma once


#pragma CIDLIB_PACK(CIDLIBPACK)


// ---------------------------------------------------------------------------
//  CLASS: TXMLCMNode
// PREFIX: xcmn
// ---------------------------------------------------------------------------
class TXMLCMNode
{
    public  :
        // -------------------------------------------------------------------
        //  Public Destructor
        // -------------------------------------------------------------------
        virtual ~TXMLCMNode();


        // -------------------------------------------------------------------
        //  Public, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TBoolean bIsNullable() const = 0;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDXML::ECMNodeTypes eNodeType() const;

        const TBitset& btsFirstPos() const;

        const TBitset& btsLastPos() const;

        tCIDLib::TCard4 c4MaxStates
        (
            const   tCIDLib::TCard4         c4ToSet
        );


    protected :
        // -------------------------------------------------------------------
        //  Hidden constructors
        // -------------------------------------------------------------------
        TXMLCMNode
        (
            const   tCIDXML::ECMNodeTypes   eNodeType
        );


        // -------------------------------------------------------------------
        //  Protected, virtual methods
        // -------------------------------------------------------------------
        virtual tCIDLib::TVoid CalculateFirst(TBitset& btsToSet) const = 0;
        virtual tCIDLib::TVoid CalculateLast(TBitset& btsToSet) const = 0;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TXMLCMNode();
        TXMLCMNode(const TXMLCMNode&);
        tCIDLib::TVoid operator=(const TXMLCMNode&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4MaxNFAStates
        //      The maximum number of states the NFA can have. We'll be told
        //      this at some point, via the c4MaxStates() method. Its required
        //      to efficiently do our calcuations.
        //
        //  m_eNodeType
        //      This type of node. This indicates what derived type it is,
        //      and lets us do tree iterative stuff without looking at the
        //      derived type.
        //
        //  m_pbtsFirst
        //  m_pbtsLast
        //      These are the bit sets for the first and last position sets.
        //      These are defaulted in upon first use.
        // -------------------------------------------------------------------
        tCIDLib::TCard4         m_c4MaxNFAStates;
        tCIDXML::ECMNodeTypes   m_eNodeType;
        mutable TBitset*        m_pbtsFirst;
        mutable TBitset*        m_pbtsLast;
};


// ---------------------------------------------------------------------------
//  CLASS: TXMLCMLeaf
// PREFIX: xcmn
// ---------------------------------------------------------------------------
class TXMLCMLeaf : public TXMLCMNode
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TXMLCMLeaf
        (
            const   tCIDLib::TCard4         c4ElemId
            , const tCIDLib::TCard4         c4StatePos = kCIDLib::c4MaxCard
        );

        ~TXMLCMLeaf();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsNullable() const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        tCIDLib::TCard4 c4ElemId() const;

        tCIDLib::TCard4 c4StatePos() const;

        tCIDLib::TCard4 c4StatePos
        (
            const   tCIDLib::TCard4         c4NewPos
        );


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CalculateFirst(TBitset& btsToSet) const;
        tCIDLib::TVoid CalculateLast(TBitset& btsToSet) const;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TXMLCMLeaf();
        TXMLCMLeaf(const TXMLCMNode&);
        tCIDLib::TVoid operator=(const TXMLCMLeaf&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_c4ElemId
        //      This is the element decl pool id of the element that this
        //      leaf node represents.
        //
        //  m_c4StatePos
        //      This is the state position assigned to this leaf by the NFA
        //      to DFA algorithm. If its c4MaxCard, then its an epsilon node,
        //      else its a non-epsilon. Epsilon nodes can be moved through
        //      without eating any input.
        // -------------------------------------------------------------------
        tCIDLib::TCard4     m_c4ElemId;
        tCIDLib::TCard4     m_c4StatePos;
};



// ---------------------------------------------------------------------------
//  CLASS: TXMLCMBinOp
// PREFIX: xcmn
// ---------------------------------------------------------------------------
class TXMLCMBinOp : public TXMLCMNode
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TXMLCMBinOp
        (
            const   tCIDXML::ECMNodeTypes   eNodeType
            ,       TXMLCMNode* const       pcmnLeftToAdopt
            ,       TXMLCMNode* const       pcmnRightToAdopt
        );

        ~TXMLCMBinOp();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsNullable() const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        TXMLCMNode* pxcmnLeft();
        const TXMLCMNode* pxcmnLeft() const;
        TXMLCMNode* pxcmnRight();
        const TXMLCMNode* pxcmnRight() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CalculateFirst(TBitset& btsToSet) const;
        tCIDLib::TVoid CalculateLast(TBitset& btsToSet) const;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TXMLCMBinOp();
        TXMLCMBinOp(const TXMLCMBinOp&);
        tCIDLib::TVoid operator=(const TXMLCMBinOp&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pxcmnLeft
        //  m_pxcmnRight
        //      These are the child nodes of this binary node. We own them
        //      and destroy them when we are destructed.
        // -------------------------------------------------------------------
        TXMLCMNode*     m_pxcmnLeft;
        TXMLCMNode*     m_pxcmnRight;
};



// ---------------------------------------------------------------------------
//  CLASS: TXMLCMUnaryOp
// PREFIX: xcmn
// ---------------------------------------------------------------------------
class TXMLCMUnaryOp : public TXMLCMNode
{
    public  :
        // -------------------------------------------------------------------
        //  Constructors and Destructor
        // -------------------------------------------------------------------
        TXMLCMUnaryOp
        (
            const   tCIDXML::ECMNodeTypes   eType
            ,       TXMLCMNode* const       pcmnToAdopt
        );

        ~TXMLCMUnaryOp();


        // -------------------------------------------------------------------
        //  Public, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TBoolean bIsNullable() const;


        // -------------------------------------------------------------------
        //  Public, non-virtual methods
        // -------------------------------------------------------------------
        TXMLCMNode* pxcmnChild();
        const TXMLCMNode* pxcmnChild() const;


    protected :
        // -------------------------------------------------------------------
        //  Protected, inherited methods
        // -------------------------------------------------------------------
        tCIDLib::TVoid CalculateFirst(TBitset& btsToSet) const;
        tCIDLib::TVoid CalculateLast(TBitset& btsToSet) const;


    private :
        // -------------------------------------------------------------------
        //  Unimplemented constructors and operators
        // -------------------------------------------------------------------
        TXMLCMUnaryOp();
        TXMLCMUnaryOp(const TXMLCMUnaryOp&);
        tCIDLib::TVoid operator=(const TXMLCMUnaryOp&);


        // -------------------------------------------------------------------
        //  Private data members
        //
        //  m_pxcmnChild
        //      This is the child node of this binary node. We own it and
        //      destroy it when we are destructed.
        // -------------------------------------------------------------------
        TXMLCMNode*     m_pxcmnChild;
};


#pragma CIDLIB_POPPACK


// ---------------------------------------------------------------------------
//  TXMLCMNode: Public, non-virtual methods
// ---------------------------------------------------------------------------
inline tCIDLib::TCard4 TXMLCMNode::c4MaxStates(const tCIDLib::TCard4 c4ToSet)
{
    m_c4MaxNFAStates = c4ToSet;
    return m_c4MaxNFAStates;
}

inline tCIDXML::ECMNodeTypes TXMLCMNode::eNodeType() const
{
    return m_eNodeType;
}

inline const TBitset& TXMLCMNode::btsFirstPos() const
{
    if (!m_pbtsFirst)
    {
        m_pbtsFirst = new TBitset(m_c4MaxNFAStates);
        CalculateFirst(*m_pbtsFirst);
    }
    return *m_pbtsFirst;
}

inline const TBitset& TXMLCMNode::btsLastPos() const
{
    if (!m_pbtsLast)
    {
        m_pbtsLast = new TBitset(m_c4MaxNFAStates);
        CalculateLast(*m_pbtsLast);
    }
    return *m_pbtsLast;
}
