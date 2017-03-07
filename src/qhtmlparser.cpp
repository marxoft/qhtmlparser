/*!
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "qhtmlparser.h"
#include <tidy.h>
#include <buffio.h>
#include <QIODevice>
#include <QRegExp>

static ctmbstr nodeAttribute(TidyNode node, const QString &name) {
    TidyAttr attr;
    
    for (attr = tidyAttrFirst(node); attr; attr = tidyAttrNext(attr)) {
        if (tidyAttrName(attr) == name) {
            return tidyAttrValue(attr);
        }
    }
    
    return 0;
}

static bool matchAttribute(const QString &value, const QHtmlAttributeMatch &match) {
    bool matches = false;
    
    if (match.testFlag(QHtmlParser::MatchExactly)) {
        matches = value.compare(match.value(), match.testFlag(QHtmlParser::MatchCaseSensitive)
                                ? Qt::CaseSensitive : Qt::CaseInsensitive) == 0;
    }
    else if (match.testFlag(QHtmlParser::MatchContains)) {
        matches = value.contains(match.value(), match.testFlag(QHtmlParser::MatchCaseSensitive)
                                 ? Qt::CaseSensitive : Qt::CaseInsensitive);
    }
    else if (match.testFlag(QHtmlParser::MatchStartsWith)) {
        matches = value.startsWith(match.value(), match.testFlag(QHtmlParser::MatchCaseSensitive)
                                   ? Qt::CaseSensitive : Qt::CaseInsensitive);

        if ((matches) && (match.testFlag(QHtmlParser::MatchEndsWith))) {
            matches = value.endsWith(match.value(), match.testFlag(QHtmlParser::MatchCaseSensitive)
                                     ? Qt::CaseSensitive : Qt::CaseInsensitive);
        }
    }
    else if (match.testFlag(QHtmlParser::MatchEndsWith)) {
        matches = value.endsWith(match.value(), match.testFlag(QHtmlParser::MatchCaseSensitive)
                                 ? Qt::CaseSensitive : Qt::CaseInsensitive);
    }
    else if (match.testFlag(QHtmlParser::MatchRegExp)) {
        matches = value.contains(QRegExp(match.value(), match.testFlag(QHtmlParser::MatchCaseSensitive)
                                 ? Qt::CaseSensitive : Qt::CaseInsensitive));
    }
    else if (match.testFlag(QHtmlParser::MatchWildcard)) {
        matches = value.contains(QRegExp(match.value(), match.testFlag(QHtmlParser::MatchCaseSensitive)
                                 ? Qt::CaseSensitive : Qt::CaseInsensitive, QRegExp::Wildcard));
    }

    return matches;
}

static bool matchAttributes(TidyNode node, const QHtmlAttributeMatches &matches, QHtmlParser::MatchType matchType) {    
    if (matchType == QHtmlParser::MatchAll) {        
        foreach (const QHtmlAttributeMatch &match, matches) {
            const ctmbstr value = nodeAttribute(node, match.name());
            
            if ((!value) || (!matchAttribute(value, match))) {
                return false;
            }
        }
            
        return true;
    }
        
    foreach (const QHtmlAttributeMatch &match, matches) {
        const ctmbstr value = nodeAttribute(node, match.name());
        
        if ((value) && (matchAttribute(value, match))) {
            return true;
        }
    }
    
    return false;
}

static TidyNode childStartNode(TidyNode node) {
    TidyNode child;
    
    for (child = tidyGetChild(node); child; child = tidyGetNext(child)) {
        switch (tidyNodeGetType(child)) {
        case TidyNode_Start:
        case TidyNode_StartEnd:
            return child;
        default:
            break;
        }
        
        childStartNode(child);
    }
    
    return 0;
}

static TidyNode previousSiblingStartNode(TidyNode node) {
    TidyNode sibling;
    
    for (sibling = tidyGetPrev(node); sibling; sibling = tidyGetPrev(sibling)) {
        switch (tidyNodeGetType(sibling)) {
        case TidyNode_Start:
        case TidyNode_StartEnd:
            return sibling;
        default:
            break;
        }        
    }
    
    return 0;
}

static TidyNode nextSiblingStartNode(TidyNode node) {
    TidyNode sibling;
    
    for (sibling = tidyGetNext(node); sibling; sibling = tidyGetNext(sibling)) {
        switch (tidyNodeGetType(sibling)) {
        case TidyNode_Start:
        case TidyNode_StartEnd:
            return sibling;
        default:
            break;
        }        
    }
    
    return 0;
}

static void allStartNodes(TidyNode node, QList<TidyNode> &nodes) {
    TidyNode child;
    
    for (child = tidyGetChild(node); child; child = tidyGetNext(child)) {
        switch (tidyNodeGetType(child)) {
        case TidyNode_Start:
        case TidyNode_StartEnd:
            nodes << child;
            break;
        default:
            break;
        }
        
        allStartNodes(child, nodes);
    }
}

static QList<TidyNode> allStartNodes(TidyNode node) {
    QList<TidyNode> nodes;
    allStartNodes(node, nodes);
    return nodes;
}

static QList<TidyNode> childStartNodes(TidyNode node) {
    QList<TidyNode> nodes;
    TidyNode child;

    for (child = childStartNode(node); child; child = nextSiblingStartNode(child)) {
        nodes << child;
    }

    return nodes;
}

static void allTextNodes(TidyNode node, QList<TidyNode> &nodes) {
    TidyNode child;
    
    for (child = tidyGetChild(node); child; child = tidyGetNext(child)) {
        switch (tidyNodeGetType(child)) {
        case TidyNode_Text:
            nodes << child;
            break;
        default:
            break;
        }
        
        allTextNodes(child, nodes);
    }
}

static QList<TidyNode> allTextNodes(TidyNode node) {
    QList<TidyNode> nodes;
    allTextNodes(node, nodes);
    return nodes;
}

QHtmlAttribute::QHtmlAttribute() {}

QHtmlAttribute::QHtmlAttribute(const QString &name, const QString &value) :
    m_name(name),
    m_value(value)
{
}

const QString& QHtmlAttribute::name() const {
    return m_name;
}

void QHtmlAttribute::setName(const QString &name) {
    m_name = name;
}

const QString& QHtmlAttribute::value() const {
    return m_value;
}

void QHtmlAttribute::setValue(const QString &value) {
    m_value = value;
}

bool QHtmlAttribute::operator==(const QHtmlAttribute &other) const {
    return (other.name() == name()) && (other.value() == value());
}

bool QHtmlAttribute::operator!=(const QHtmlAttribute &other) const {
    return (other.name() != name()) || (other.value() != value());
}

QHtmlAttributeMatch::QHtmlAttributeMatch() :
    QHtmlAttribute(),
    m_flags(QHtmlParser::MatchFlags(QHtmlParser::MatchExactly))
{
}

QHtmlAttributeMatch::QHtmlAttributeMatch(const QString &name, const QString &value, QHtmlParser::MatchFlags flags) :
    QHtmlAttribute(name, value),
    m_flags(flags)
{
}

QHtmlParser::MatchFlags QHtmlAttributeMatch::flags() const {
    return m_flags;
}

void QHtmlAttributeMatch::setFlags(QHtmlParser::MatchFlags flags) {
    m_flags = flags;
}

void QHtmlAttributeMatch::setFlag(QHtmlParser::MatchFlag flag, bool on) {
    if (on) {
        m_flags |= flag;
    }
    else {
        m_flags &= ~flag;
    }
}

bool QHtmlAttributeMatch::testFlag(QHtmlParser::MatchFlag flag) const {
    return m_flags.testFlag(flag);
}

bool QHtmlAttributeMatch::operator==(const QHtmlAttributeMatch &other) const {
    return (other.name() == name()) && (other.value() == value()) && (other.flags() == flags());
}

bool QHtmlAttributeMatch::operator!=(const QHtmlAttributeMatch &other) const {
    return (other.name() != name()) || (other.value() != value()) || (other.flags() != flags());
}

class QHtmlElementPrivate
{

public:
    QHtmlElementPrivate() :
        document(0),
        node(0)
    {
    }
    
    ~QHtmlElementPrivate() {}    
    
    TidyDoc document;
    TidyNode node;
};

QHtmlElement::QHtmlElement() :
    d(new QHtmlElementPrivate)
{
}

QHtmlElement::QHtmlElement(const QHtmlElement &other) :
    d(new QHtmlElementPrivate)
{
    d->document = other.d->document;
    d->node = other.d->node;
}

QHtmlElement::~QHtmlElement() {
    delete d;
}

QHtmlAttributes QHtmlElement::attributes() const {
    QHtmlAttributes attrs;
    
    if (!d->node) {
        return attrs;
    }
    
    TidyAttr attr;
    
    for (attr = tidyAttrFirst(d->node); attr; attr = tidyAttrNext(attr)) {
        attrs << QHtmlAttribute(tidyAttrName(attr), tidyAttrValue(attr));
    }
    
    return attrs;
}

QString QHtmlElement::attribute(const QString &name) const {
    if (!d->node) {
        return QString();
    }
    
    const ctmbstr value = nodeAttribute(d->node, name);
    
    if (value) {
        return value;
    }
    
    return QString();
}

QHtmlElement QHtmlElement::parentElement() const {
    QHtmlElement element;

    if (!d->node) {
        return element;
    }

    TidyNode node = tidyGetParent(d->node);

    if (node) {
        element.d->document = d->document;
        element.d->node = node;
    }

    return element;
}

QHtmlElement QHtmlElement::nextSibling() const {
    QHtmlElement element;

    if (!d->node) {
        return element;
    }

    TidyNode node = nextSiblingStartNode(d->node);

    if (node) {
        element.d->document = d->document;
        element.d->node = node;
    }

    return element;
}

QHtmlElement QHtmlElement::previousSibling() const {
    QHtmlElement element;

    if (!d->node) {
        return element;
    }

    TidyNode node = previousSiblingStartNode(d->node);

    if (node) {
        element.d->document = d->document;
        element.d->node = node;
    }

    return element;
}

QHtmlElementList QHtmlElement::childElements() const {
    QHtmlElementList elements;
    
    if (!d->node) {
        return elements;
    }
    
    foreach (TidyNode node, childStartNodes(d->node)) {
        QHtmlElement element;
        element.d->document = d->document;
        element.d->node = node;
        elements << element;
    }
    
    return elements;
}

QHtmlElement QHtmlElement::firstChildElement() const {
    QHtmlElement element;
    
    if (!d->node) {
        return element;
    }
    
    TidyNode node = childStartNode(d->node);
    
    if (node) {
        element.d->document = d->document;
        element.d->node = node;
    }
    
    return element;
}

QHtmlElement QHtmlElement::lastChildElement() const {
    QHtmlElement element;
    
    if (!d->node) {
        return element;
    }
    
    const QList<TidyNode> nodes = childStartNodes(d->node);
    
    if (!nodes.isEmpty()) {
        element.d->document = d->document;
        element.d->node = nodes.last();
    }
    
    return element;
}

QHtmlElement QHtmlElement::nthChildElement(int n) const {
    if (n == 0) {
        return firstChildElement();
    }
    
    QHtmlElement element;
    
    if (!d->node) {
        return element;
    }
    
    const QList<TidyNode> nodes = childStartNodes(d->node);
    
    if (n < 0) {
        if (nodes.size() + n >= 0) {
            element.d->document = d->document;
            element.d->node = nodes.at(nodes.size() + n);
        }
    }
    else if (n < nodes.size() - 1) {
        element.d->document = d->document;
        element.d->node = nodes.at(n);
    }

    return element;
}

QHtmlElement QHtmlElement::elementById(const QString &id) const {
    QHtmlElement element;
    
    if (!d->node) {
        return element;
    }
    
    foreach (TidyNode node, allStartNodes(d->node)) {
        ctmbstr value = nodeAttribute(node, "id");   

        if ((value) && (value == id)) {
            element.d->document = d->document;
            element.d->node = node;
            break;
        }
    }
    
    return element;
}

QHtmlElementList QHtmlElement::elementsByTagName(const QString &name) const {
    QHtmlElementList elements;
    
    if (!d->node) {
        return elements;
    }
    
    foreach (TidyNode node, allStartNodes(d->node)) {
        if (tidyNodeGetName(node) == name) {
            QHtmlElement element;
            element.d->document = d->document;
            element.d->node = node;
            elements << element;
        }
    }

    return elements;
}

QHtmlElementList QHtmlElement::elementsByTagName(const QString &name, const QHtmlAttributeMatch &match) const {
    return elementsByTagName(name, QHtmlAttributeMatches() << match);
}

QHtmlElementList QHtmlElement::elementsByTagName(const QString &name, const QHtmlAttributeMatches &matches,
                                                 QHtmlParser::MatchType matchType) const {
    QHtmlElementList elements;
    
    if (!d->node) {
        return elements;
    }
    
    foreach (TidyNode node, allStartNodes(d->node)) {
        if ((tidyNodeGetName(node) == name) && (matchAttributes(node, matches, matchType))) {
            QHtmlElement element;
            element.d->document = d->document;
            element.d->node = node;
            elements << element;
        }
    }
    
    return elements;
}

QHtmlElement QHtmlElement::firstElementByTagName(const QString &name) const {
    QHtmlElement element;
    
    if (!d->node) {
        return element;
    }
    
    foreach (TidyNode node, allStartNodes(d->node)) {
        if (tidyNodeGetName(node) == name) {
            element.d->document = d->document;
            element.d->node = node;
            break;
        }
    }
    
    return element;
}

QHtmlElement QHtmlElement::firstElementByTagName(const QString &name, const QHtmlAttributeMatch &match) const {
    return firstElementByTagName(name, QHtmlAttributeMatches() << match);
}

QHtmlElement QHtmlElement::firstElementByTagName(const QString &name, const QHtmlAttributeMatches &matches,
                                                 QHtmlParser::MatchType matchType) const {
    QHtmlElement element;
    
    if (!d->node) {
        return element;
    }
    
    foreach (TidyNode node, allStartNodes(d->node)) {
        if ((tidyNodeGetName(node) == name) && (matchAttributes(node, matches, matchType))) {
            element.d->document = d->document;
            element.d->node = node;
            break;
        }
    }
    
    return element;
}

QHtmlElement QHtmlElement::lastElementByTagName(const QString &name) const {
    const QHtmlElementList elements = elementsByTagName(name);

    if (!elements.isEmpty()) {
        return elements.last();
    }

    return QHtmlElement();
}

QHtmlElement QHtmlElement::lastElementByTagName(const QString &name, const QHtmlAttributeMatch &match) const {
    const QHtmlElementList elements = elementsByTagName(name, match);

    if (!elements.isEmpty()) {
        return elements.last();
    }

    return QHtmlElement();
}

QHtmlElement QHtmlElement::lastElementByTagName(const QString &name, const QHtmlAttributeMatches &matches,
                                                QHtmlParser::MatchType matchType) const {
    const QHtmlElementList elements = elementsByTagName(name, matches, matchType);

    if (!elements.isEmpty()) {
        return elements.last();
    }

    return QHtmlElement();
}

QHtmlElement QHtmlElement::nthElementByTagName(int n, const QString &name) const {
    if (n == 0) {
        return firstElementByTagName(name);
    }
    
    QHtmlElement element;
    
    if (!d->node) {
        return element;
    }
    
    const QList<TidyNode> nodes = allStartNodes(d->node);

    if (nodes.isEmpty()) {
        return element;
    }

    const int start = (n < 0 ? nodes.size() - 1 : 0);
    const int end = (n < 0 ? 0 : nodes.size() - 1);
    int inc = (n < 0 ? -1 : 1);
    int hits = 0;

    for (int i = start; i != end; i += inc) {
        if (tidyNodeGetName(nodes.at(i)) == name) {
            if (hits == n) {
                element.d->document = d->document;
                element.d->node = nodes.at(i);
                break;
            }

            hits += inc;
        }
    }
    
    return element;
}

QHtmlElement QHtmlElement::nthElementByTagName(int n, const QString &name, const QHtmlAttributeMatch &match) const {
    return nthElementByTagName(n, name, QHtmlAttributeMatches() << match);
}

QHtmlElement QHtmlElement::nthElementByTagName(int n, const QString &name, const QHtmlAttributeMatches &matches,
                                               QHtmlParser::MatchType matchType) const {
    if (n == 0) {
        return firstElementByTagName(name, matches, matchType);
    }
    
    QHtmlElement element;
    
    if (!d->node) {
        return element;
    }
    
    const QList<TidyNode> nodes = allStartNodes(d->node);

    if (nodes.isEmpty()) {
        return element;
    }
    
    const int start = (n < 0 ? nodes.size() - 1 : 0);
    const int end = (n < 0 ? 0 : nodes.size() - 1);
    int inc = (n < 0 ? -1 : 1);
    int hits = 0;

    for (int i = start; i != end; i += inc) {
        if ((tidyNodeGetName(nodes.at(i)) == name) && (matchAttributes(nodes.at(i), matches, matchType))) {
            if (hits == n) {
                element.d->document = d->document;
                element.d->node = nodes.at(i);
                break;
            }

            hits += inc;
        }
    }
    
    return element;
}

QString QHtmlElement::tagName() const {
    if (d->node) {
        return tidyNodeGetName(d->node);
    }
    
    return QString();
}

QString QHtmlElement::text(bool includeChildElements) const {    
    if ((!d->document) || (!d->node)) {
        return QString();
    }
    
    TidyBuffer buffer = TidyBuffer();

    if (includeChildElements) {
        foreach (TidyNode node, allTextNodes(d->node)) {
            tidyNodeGetText(d->document, node, &buffer);
        }
    }
    else {
        foreach (TidyNode node, allTextNodes(d->node)) {
            if (tidyGetParent(node) == d->node) {
                tidyNodeGetText(d->document, node, &buffer);
                break;
            }
        }
    }
    
    if (buffer.bp) {
        QString text = QString::fromUtf8((char*)buffer.bp);
        tidyBufFree(&buffer);

        if (text.endsWith("\n")) {
            text.chop(1);
        }

        return text;
    }
    
    return QString();
}

QString QHtmlElement::toString() const {    
    if ((!d->document) || (!d->node)) {
        return QString();
    }
    
    TidyBuffer buffer = TidyBuffer();
    
    if (tidyNodeGetText(d->document, d->node, &buffer)) {
        QString text = QString::fromUtf8((char *)buffer.bp);
        tidyBufFree(&buffer);        
        return text.trimmed();
    }
    
    return QString();
}

bool QHtmlElement::isNull() const {
    return (!d->document) || (!d->node);
}

QHtmlElement& QHtmlElement::operator=(const QHtmlElement &other) {
    d->document = other.d->document;
    d->node = other.d->node;
    return *this;
}

bool QHtmlElement::operator==(const QHtmlElement &other) const {
    return (other.d->document == d->document) && (other.d->node == d->node);
}

bool QHtmlElement::operator!=(const QHtmlElement &other) const {
    return (other.d->document != d->document) || (other.d->node != d->node);
}

class QHtmlDocumentPrivate
{

public:
    QHtmlDocumentPrivate() :
        document(0),
        error(false)
    {
    }
    
    ~QHtmlDocumentPrivate() {
        if (document) {
            tidyRelease(document);
        }
    }
    
    bool setContent(const QByteArray &content) {
        if (document) {
            tidyRelease(document);
        }
        
        document = tidyCreate();
        tidySetCharEncoding(document, "utf8");
        tidyOptSetBool(document, TidyForceOutput, yes);
        tidyOptSetInt(document, TidyWrapLen, 0);
        tidyOptSetBool(document, TidyQuiet, yes);
        tidyOptSetBool(document, TidyShowWarnings, no);
        
        TidyBuffer errorBuffer = TidyBuffer();
        tidySetErrorBuffer(document, &errorBuffer);
        tidyParseString(document, content.constData());
        error = tidyErrorCount(document) > 0;
        
        if (error) {
            errorString = QString::fromUtf8((char*)errorBuffer.bp);
            tidyBufFree(&errorBuffer);
        }
        else {
            errorString = QString();
        }
        
        return !error;
    }
    
    TidyDoc document;
    
    bool error;
    QString errorString;
};

QHtmlDocument::QHtmlDocument() :
    d(new QHtmlDocumentPrivate)
{
}

QHtmlDocument::QHtmlDocument(const QString &content) :
    d(new QHtmlDocumentPrivate)
{
    setContent(content);
}

QHtmlDocument::QHtmlDocument(const QByteArray &content) :
    d(new QHtmlDocumentPrivate)
{
    setContent(content);
}

QHtmlDocument::QHtmlDocument(QIODevice *device) :
    d(new QHtmlDocumentPrivate)
{
    setContent(device);
}

QHtmlDocument::~QHtmlDocument() {
    delete d;
}

bool QHtmlDocument::setContent(const QString &content) {
    return d->setContent(content.toUtf8());
}

bool QHtmlDocument::setContent(const QByteArray &content) {
    return d->setContent(content);
}

bool QHtmlDocument::setContent(QIODevice *device) {
    if (device) {
        return d->setContent(device->readAll());
    }
    
    return false;
}

QHtmlElement QHtmlDocument::documentElement() const {
    QHtmlElement element;
    
    if (!d->document) {
        return element;
    }
    
    TidyNode root = tidyGetRoot(d->document);
    
    if (root) {
        element.d->document = d->document;
        element.d->node = root;
    }
    
    return element;
}

QHtmlElement QHtmlDocument::htmlElement() const {
    QHtmlElement element;
    
    if (!d->document) {
        return element;
    }
    
    TidyNode html = tidyGetHtml(d->document);
    
    if (html) {
        element.d->document = d->document;
        element.d->node = html;
    }
    
    return element;
}

QHtmlElement QHtmlDocument::headElement() const {
    QHtmlElement element;
    
    if (!d->document) {
        return element;
    }
    
    TidyNode head = tidyGetHead(d->document);
    
    if (head) {
        element.d->document = d->document;
        element.d->node = head;
    }
    
    return element;
}

QHtmlElement QHtmlDocument::bodyElement() const {
    QHtmlElement element;
    
    if (!d->document) {
        return element;
    }
    
    TidyNode body = tidyGetBody(d->document);
    
    if (body) {
        element.d->document = d->document;
        element.d->node = body;
    }
    
    return element;
}

QString QHtmlDocument::toString() const {    
    if (!d->document) {
        return QString();
    }
    
    TidyBuffer buffer = TidyBuffer();
    
    if (tidySaveBuffer(d->document, &buffer) >= 0) {
        QString text = QString::fromUtf8((char *)buffer.bp);
        tidyBufFree(&buffer);
        return text;
    }
    
    return QString();
}

bool QHtmlDocument::hasError() const {
    return d->error;
}

QString QHtmlDocument::errorString() const {
    return d->errorString;
}

bool QHtmlDocument::isNull() const {
    return d->document ? false : true;
}
