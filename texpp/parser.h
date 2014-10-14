/*  This file is part of texpp library.
    Copyright (C) 2009 Vladimir Kuznetsov <ks.vladimir@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __TEXPP_PARSER_H
#define __TEXPP_PARSER_H

#include <texpp/common.h>
#include <texpp/lexer.h>
#include <texpp/command.h>
#include <texpp/command.h>

#include <deque>
#include <set>
#include <cassert>
#include <climits>

#include <boost/any.hpp>

namespace texpp {

using boost::any;
using boost::any_cast;
using boost::unsafe_any_cast;

class Lexer;
class Logger;
class Parser;

namespace base {
    class ExpandafterMacro;
} // namespace base

class Node
{
public:
    typedef shared_ptr<Node> ptr;
    typedef vector< pair< string, Node::ptr > > ChildrenList;

    Node(const string& type): m_type(type) {}

    /**
     * @brief source - recover input source TeX file for this node
     * @param fileName - name of input TeX source file. Tokens in this node wich
     *      came from file with different name will not be represented here
     * @return string whitch is initial input TeX text within this actual node
     */
    string source(const string& fileName = string()) const;
    unordered_map<shared_ptr<string>, string> sources() const;
    std::set<shared_ptr<string> > files() const;
    shared_ptr<string> oneFile() const;
    bool isOneFile() const;

    // Returns a pair (start_pos, end_pos)
    std::pair<size_t, size_t> sourcePos() const;

    const string& type() const { return m_type; }
    void setType(const string& type) { m_type = type; }

    void setValue(const any& value) { m_value = value; }

    const any& valueAny() const { return m_value; }

    /** return def       if types of def and m_value are different
     *  return m_value   if types of def and m_value are equal
     */
    template<typename T>
    T value(T def) const {
        if(m_value.type() != typeid(T)) return def;
        else return *unsafe_any_cast<T>(&m_value);
    }

    // XXX: The following is a horrible hack required to
    // XXX: overcome problems with RTTI across shared objects
    // XXX: It will never work good and should be removed !
    // XXX: The only solution is to NOT use boost::any
    const string& valueString() const;

    const vector< Token::ptr >& tokens() const { return m_tokens; }
    vector< Token::ptr >& tokens() { return m_tokens; }

    const ChildrenList& children() const { return m_children; }
    ChildrenList& children() { return m_children; }

    size_t childrenCount() const { return m_children.size(); }
    Node::ptr child(int num) { return m_children[num].second; }
    Node::ptr child(const string& name);

    /**
     * @brief append node to m_children list with tag "name"
     * @param name - tag of node
     */
    void appendChild(const string& name, Node::ptr node) {
        m_children.push_back(make_pair(name, node));
    }

    /**
     * return last real token from Node-token tree;
     * return empty token if no token-tree is emppty
     */
    Token::ptr lastToken();

    /**
     * @brief representing node in string format
     * @return string in format Node(<m_type> <m_value>)
     */
    string repr() const;

    /**
     * @brief represent token tree nodes
     * @param indent - token level of the hierarchy; 0 - is top level
     * @return text for representing
     */
    string treeRepr(size_t indent = 0) const;

protected:
    string                  m_type;     // type of token inside
    any                     m_value;    // main object in the node
    vector< Token::ptr >    m_tokens;   // set of tokens inside node.
    ChildrenList            m_children; // token node list one level lower
                                        // in the hierarchy
};

class Parser
{
public:
    enum Interaction { ERRORSTOPMODE,   // do not stop when error
                       SCROLLMODE,
                       NONSTOPMODE,
                       BATCHMODE };

    // list of modes for execution processor
    enum Mode { NULLMODE,
                VERTICAL,   // vertical lists are broken into pages
                HORIZONTAL, // horizontal lists are broken into paragraphs
                RVERTICAL,
                RHORIZONTAL,
                MATH,       // formulas are built out of math lists
                DMATH };
    enum GroupType { GROUP_DOCUMENT,    // scope - the document, highest scope
                     GROUP_NORMAL,      // inside curve brackets {...}
                     GROUP_SUPER,       // inside quotes "..."
                     GROUP_MATH,        // inside formula $...$
                     GROUP_DMATH,
                     GROUP_CUSTOM };

    Parser(const string& fileName, std::istream* file,
            const string& workdir = string(),
            bool interactive = false, bool ignoreEmergency = false,
            shared_ptr<Logger> logger = shared_ptr<Logger>());

    Parser(const string& fileName, shared_ptr<std::istream> file,
            const string& workdir = string(),
            bool interactive = false, bool ignoreEmergency = false,
            shared_ptr<Logger> logger = shared_ptr<Logger>());

    Interaction interaction() const { return m_interaction; }
    void setInteraction(Interaction intr) { m_interaction = intr; }

    const string& workdir() const { return m_workdir; }
    void setWorkdir(const string& workdir) { m_workdir = workdir; }

    bool ignoreEmergency() const { return m_ignoreEmergency; }
    void setIgnoreEmergency(bool ignoreEmergency) {
        m_ignoreEmergency = ignoreEmergency;
    }
   
    ///////// Parse 
    Node::ptr parse();

    const string& modeName() const;
    Mode mode() const { return m_mode; }
    void setMode(Mode mode) { m_mode = mode; }

    bool hasOutput() const { return m_hasOutput; }

    /**
     * @brief if "tracingcommands" control command was set to positive int ( >0)
     * @param token
     * @param expanding
     */
    void traceCommand(Token::ptr token, bool expanding = false);

    //////// Tokens

    /**
     * @brief lastToken return the last not skiped token
     * @return pointer to the last not skiped token
     */
    Token::ptr lastToken();

    /**
     * @brief puts forthcoming tokens till first real token into m_tokenSourse
     * list. Return first real token
     * @expand
     * @return first forthcoming real(no skipped) token
     */
    Token::ptr peekToken(bool expand = true);

    /**
     * @brief   insert tokens from m_tokenSource to tokenVector,
     *          update m_lineNo
     *          clean m_tokenSource and m_token
     * @return  m_token value if m_tokenSource is not empty. Otherwise use
     *          peekToken() to get next real token
     */
    Token::ptr nextToken(vector< Token::ptr >* tokenVector = NULL,
                         bool expand = true);

    /**
     * @brief copy tokens from m_tokenSource with tokenVector to m_tokenQueue;
     *        clean m_tokenSource and m_token
     */
    void pushBack(vector< Token::ptr >* tokenVector);

    // void setNoexpand(Token::ptr token) { m_noexpandToken = token; }
    void addNoexpand(Token::ptr token) { m_noexpandTokens.insert(token); }

    /**
     * @brief move tokens from m_tokenSource to m_tokenQueue
     *      clear m_noexpandTokens, m_tokenSource and m_token
     */
    void resetNoexpand() { m_noexpandTokens.clear(); pushBack(NULL); }

    void input(const string& fileName, const string& fullName);
    void end() { m_end = true; }
    void endinput() { m_endinput = true; }

    Command::ptr currentCommand() const {
        return m_commandStack.empty() ? Command::ptr() : m_commandStack.back();
    }

    Command::ptr prevCommand(size_t n = 1) const {
        return m_commandStack.size() <= n ? Command::ptr() :
            m_commandStack[m_commandStack.size()-1-n];
    }

    void lockToken(Token::ptr token) { m_lockToken = token; }
    void setAfterassignmentToken(Token::ptr t) { m_afterassignmentToken = t; }
    void addAftergroupToken(Token::ptr t) {
        if(m_groupLevel > 0) m_aftergroupTokensStack.back().push_back(t);
    }

    //////// Parse helpers
    bool helperIsImplicitCharacter(Token::CatCode catCode,
                                        bool expand = true);

    Node::ptr parseGroup(GroupType groupType);

    Node::ptr parseCommand(Command::ptr command);

    Node::ptr parseToken(bool expand = true);
    Node::ptr parseDMathToken();
    Node::ptr parseControlSequence(bool expand = true);

    Node::ptr parseOptionalSpaces();

    Node::ptr parseKeyword(const vector<string>& keywords);
    Node::ptr parseOptionalKeyword(const vector<string>& keywords);

    Node::ptr parseOptionalEquals();
    Node::ptr parseOptionalSigns();
    Node::ptr parseNormalInteger();
    Node::ptr parseNumber();
    Node::ptr parseDimenFactor();
    Node::ptr parseNormalDimen(bool fil = false, bool mu = false);
    Node::ptr parseDimen(bool fil = false, bool mu = false);
    Node::ptr parseGlue(bool mu = false);

    Node::ptr parseFiller(bool expand);
    Node::ptr parseBalancedText(bool expand, int paramCount = -1,
                                    Token::ptr nameToken = Token::ptr());
    Node::ptr parseGeneralText(bool expand, bool implicitLbrace = true);

    Node::ptr parseFileName();

    /**
     * @brief read word untill non letter symbol
     * @return node with the word inside
     */
    Node::ptr parseTextWord();

    Node::ptr parseTextCharacter();

    /**
     * @brief set the HORIZONTAL mode. Insert "ch" to the to m_symbols table
     * @param ch - symbol
     * @param token
     */
    void processTextCharacter(char ch, Token::ptr token);
    void resetParagraphIndent();

    //////// Symbols
    /**
     * @brief insert <value> to m_symbols table with tag <name>
     * @param name - tag for <value>
     * @param value - value to be inserted to m_symbols table
     * @param global: false - set valid only within current scope,
     *                true  - set valid for a whole document
     */
    void setSymbol(const string& name, const any& value, bool global = false);

    /** @brief in case the token is control command - register token's symbol
     *      combination(command) in m_symbols table
     *  @param token - source for tag. tag is token's semantic
     *  @param value - value to be inserted to m_symbols table
     *  @param global: false - set valid only within current scope,
     *                 true  - set valid for a whole document
     */
    void setSymbol(Token::ptr token, const any& value, bool global = false) {
        if(token && token->isControl())
            setSymbol(token->value(), value, global);
    }

    void setSymbolDefault(const string& name, const any& defaultValue);
    
    const any& symbolAny(const string& name) const;
    const any& symbolAny(Token::ptr token) const {
        if(!token || !token->isControl()) return EMPTY_ANY;
        else return symbolAny(token->value());
    }
    
    template<typename T>
    T symbol(const string& name, T def) const {
        const any& v = symbolAny(name);
        if(v.type() != typeid(T)) return def;
        else return *unsafe_any_cast<T>(&v);
    }

    template<typename T>
    T symbol(Token::ptr token, T def) const {
        const any& v = symbolAny(token);
        if(v.type() != typeid(T)) return def;
        else return *unsafe_any_cast<T>(&v);
    }

    template<typename T>
    shared_ptr<T> symbolCommand(Token::ptr token) const {
        return dynamic_pointer_cast<T>(
                symbol(token, Command::ptr()));
    }

    void beginGroup();
    void endGroup();
    int groupLevel() const { return m_groupLevel; }

    void beginCustomGroup(const string& type) {
        m_customGroupBegin = true; m_customGroupType = type; beginGroup(); }
    void endCustomGroup() { endGroup(); m_customGroupEnd = true; }

    /**
     * @brief return escape character
     * @return escape character in string format
     */
    // TODO: manage e==0 case
    string escapestr() const {
        int e = symbol("escapechar", int(0));
        return e >= 0 && e <= 255 ? string(1, e) : string();
    }

    //////// Others
    shared_ptr<Logger> logger() { return m_logger; }
    shared_ptr<Lexer> lexer() { return m_lexer; }

    static const string& banner() { return BANNER; }

protected:
    void endinputNow();


    // TODO: this method is huge. So, documentation should be complited by time
    /**
     * @brief rawExpandToken make expading of command. If the command is not
     *      a Macro - return null Node pointer...
     * @param token - command token to be expanded.
     * @return null Node pointer if command is not a Macro. Otherwise return
     * expanding of Macro...
     */
    Node::ptr rawExpandToken(Token::ptr token);

    /**
     * @brief read and return next token be it skipped or no
     * @return next token
     */
    Token::ptr rawNextToken(bool expand = true);
    Node::ptr parseFalseConditional(size_t level,
                          bool sElse = false, bool sOr = false);
    void setSpecialSymbol(const string& name, const any& value);

    /**
     * @brief initialising of m_symbols by control comands and control
     * variables. Filling m_catCodeTable lookup table for all 256 possible char
     * values (char <-> category code). Appending time to the banner.
     */
    void init();

    typedef std::deque<
        Token::ptr
    > TokenQueue;

    typedef std::set<
        Token::ptr
    > TokenSet;

    typedef std::vector<
        Command::ptr
    > CommandStack;

    typedef std::vector<
        pair<shared_ptr<Lexer>, TokenQueue>
    > InputStack;

    string          m_workdir;
    bool            m_ignoreEmergency;

    shared_ptr<Lexer>   m_lexer;
    shared_ptr<Logger>  m_logger;

    Token::ptr      m_token;        // current token (in process)
    Token::list     m_tokenSource;  // token "history" with actuand token at
                                    // the end if list

    Token::ptr      m_lastToken;    // the last not skiped token
    TokenSet        m_noexpandTokens;
    TokenQueue      m_tokenQueue;   // token's buffer whitch is top priority for
                                    // rawNextToken() to get token

    int             m_groupLevel;
    bool            m_end;
    bool            m_endinput;
    bool            m_endinputNow;

    struct ConditionalInfo {
        bool parsed;
        bool ifcase;
        bool active;
        int  value;
        int  branch;
    };
    vector<ConditionalInfo> m_conditionals;

    typedef unordered_map<
        string, pair< int, any >
    > SymbolTable;

    typedef vector<
        pair<string, pair<int, any> >
    > SymbolStack;

    SymbolTable     m_symbols;      // dictionary of known symbols and commands
    SymbolStack     m_symbolsStack;
    vector<size_t>  m_symbolsStackLevels;

    size_t          m_lineNo;   // current line number in file
    Mode            m_mode;     // current mode for TeXpp's "state automat"
    Mode            m_prevMode; // previous mode for TeXpp's "state automat"

    bool            m_hasOutput;

    GroupType       m_currentGroupType; // current group type

    string  m_customGroupType;
    bool    m_customGroupBegin;
    bool    m_customGroupEnd;

    CommandStack m_commandStack;

    Interaction m_interaction;
    
    Token::ptr          m_lockToken;
    Token::ptr          m_afterassignmentToken;
    vector<Token::list> m_aftergroupTokensStack;

    InputStack m_inputStack;

    static any EMPTY_ANY;
    static string BANNER;

    friend class base::ExpandafterMacro;
};

} // namespace texpp

#endif

