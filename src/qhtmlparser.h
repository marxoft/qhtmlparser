/*!
 * \file qhtmlparser.h
 *
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

#ifndef QHTMLPARSER_H
#define QHTMLPARSER_H

#include <QList>
#include <QString>

#if defined(QHTMLPARSER_LIBRARY)
#define QHTMLPARSER_EXPORT Q_DECL_EXPORT
#elif defined(QHTMLPARSER_STATIC_LIBRARY)
#define QHTMLPARSER_EXPORT
#else
#define QHTMLPARSER_EXPORT Q_DECL_IMPORT
#endif

/*!
 * \mainpage notitle
 *
 * \section intro Introduction
 *
 * QHtmlParser is a Qt/C++ library for parsing, traversing and searching HTML documents.
 *
 * QHtmlParser provides the following classes:
 *
 * <table>
 *     <tr>
 *         <th>Name</th>
 *         <th>Description</th>
 *     </tr>
 *     <tr>
 *         <td>QHtmlAttribute</td>
 *         <td>Represents an individual HTML attribute with a name and value.</td>
 *     </tr>
 *     <tr>
 *         <td>QHtmlAttributeMatch</td>
 *         <td>Used for performing a match against the attributes of an element.</td>
 *     </tr>
 *     <tr>
 *         <td>QHtmlDocument</td>
 *         <td>Used for loading and parsing a HTML document.</td>
 *     </tr>
 *     <tr>
 *         <td>QHtmlElement</td>
 *         <td>Represents an individual HTML element/tag in a document.</td>
 *     </tr>
 * </table>
 * 
 * \subsection usage Usage
 *
 * Before the elements of a HTML document can be accessed, the document must first be parsed using QHtmlDocument. 
 * This can be done by passing the document to the QHtmlDocument constructor, or by using the setContent() method. 
 * The document can be in the form of a QByteArray, QString or a QIODevice that is open for reading, e.g:
 *
 * \code
 * QFile file("doc.html");
 * file.open(QFile::ReadOnly);
 * QHtmlDocument document;
 * const bool ok = document.setContent(&file);
 *
 * if (ok) {
 *     // traverse document
 * }
 * else {
 *     // handle error
 * }
 * \endcode
 *
 * Once the document is parsed, the elements and attributes of the document may be accessed using the methods of the 
 * QHtmlElement class. In addition to basic traversal, QHtmlElement has several methods that allow you to search for 
 * elements by tag name and attributes. The following code snippet shows a search for 'div' elements with a 'class' 
 * attribute equal to 'foo' or 'bar':
 *
 * \code
 * const QHtmlElement html = document.htmlElement();
 * const QHtmlAttributeMatch match1("class", "foo");
 * const QHtmlAttributeMatch match2("class", "bar");
 * const QHtmlAttributeMatches matches = QHtmlAttributeMatches() << match1 << match2;
 * const QHtmlElementList divs = html.elementsByTagName("div", matches, QHtmlParser::MatchAny);
 *
 * foreach (const QHtmlElement &div, divs) {
 *     // process element
 * }
 * \endcode
 *
 * \subsection source Source Code
 *
 * The source code can be found at <a href="https://github.com/marxoft/qhtmlparser">GitHub</a>.
 */

/*!
 * The QHtmlParser namespace.
 */
namespace QHtmlParser
{
    /*!
     * Specifies the criteria applied when matching attribute values.
     */
    enum MatchFlag {
        /*!
         * The attribute value is equal to the specified string.
         */
        MatchExactly = 0x0000,

        /*!
         * The attribute value contains the specified string.
         */
        MatchContains = 0x0001,
        
        /*!
         * The attribute value starts with the specified string.
         */
        MatchStartsWith = 0x0002,
        
        /*!
         * The attribute value ends with the specified string.
         */
        MatchEndsWith = 0x0004,
        
        /*!
         * The attribute value matches the specified regular expression pattern.
         */
        MatchRegExp = 0x0008,
        
        /*!
         * The attribute matches the specified wildcard.
         */
        MatchWildcard = 0x0010,
        
        /*!
         * The match is case sensitive.
         */
        MatchCaseSensitive = 0x0020
    };

    /*!
     * Typedef for QFlags<MatchFlag>.
     */
    typedef QFlags<MatchFlag> MatchFlags;

    /*!
     * Specifies the criteria applied when matching a list of attributes.
     */
    enum MatchType {
        /*!
         * All matches in the list must be successful.
         */
        MatchAll = 0,

        /*!
         * Only one match in the list must be successful.
         */
        MatchAny = 1
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS(QHtmlParser::MatchFlags)

/*!
 * Represents a HTML attribute with a name and value.
 */
class QHTMLPARSER_EXPORT QHtmlAttribute
{

public:
    /*!
     * Constructs a QHtmlAttribute with an empty name and value.
     */
    QHtmlAttribute();
    
    /*!
     * Constructs a QHtmlAttribute using the specified \a name and \a value.
     */
    explicit QHtmlAttribute(const QString &name, const QString &value);
    
    /*!
     * Returns the name of the attribute.
     */
    const QString& name() const;
    
    /*!
     * Sets the name of the attribute to \a name.
     */
    void setName(const QString &name);
    
    /*!
     * Returns the value of the attribute.
     */
    const QString& value() const;
    
    /*!
     * Sets the value of the attribute to \a value.
     */
    void setValue(const QString &value);

    /*!
     * Returns \c true if both name() and value() of \a other are equal to those of this QHtmlAttribute.
     */
    bool operator==(const QHtmlAttribute &other) const;

    /*!
     * Returns \c true if either name() or value() of \a other are not equal to those of this QHtmlAttribute.
     */
    bool operator!=(const QHtmlAttribute &other) const;

private:
    QString m_name;
    QString m_value;
};

/*!
 * Defines the criteria used for performing a match against the attributes of an element.
 *
 * The QHtmlAttributeMatch class is used to define the criteria that is 
 * applied when performing a match against the attributes of a QHtmlElement.
 *
 * A match is performed by looking for any attribute of the QHtmlElement that 
 * has the same name as the QHtmlAttributeMatch, and a value that matches the 
 * criteria defined by value() and flags(). The value may be a string or 
 * regular expression pattern. The manner in which the value is matched 
 * against attributes is determined by the flags, which may be a bitwise OR 
 * combination of the QHtmlParser::MatchFlag enum values.
 *
 * Example usage:
 *
 * \code
 * QFile file("document.html");
 * file.open(QFile::ReadOnly);
 * const QHtmlDocument document(&file);
 * const QHtmlElement root = document.documentElement();
 * const QHtmlAttributeMatch classMatch("class", "foo", QHtmlParser::MatchStartsWith | QHtmlParser::MatchCaseSensitive);
 * const QHtmlAttributeMatch dataMatch("data-foo", "bar", QHtmlParser::MatchContains);
 * const QHtmlAttributeMatches matches = QHtmlAttributeMatches() << classMatch << dataMatch;
 * const QHtmlElementList elements = root.elementsByTagName("div", matches);
 *
 * foreach (const QHtmlElement &element, elements) {
 *     // process element
 * }
 * \endcode
 */
class QHTMLPARSER_EXPORT QHtmlAttributeMatch : public QHtmlAttribute
{

public:
    /*!
     * Constructs a QHtmlAttributeMatch with an empty name and value.
     */
    QHtmlAttributeMatch();
    
    /*!
     * Constructs a QHtmlAttributeMatch using the specified \a name, \a value and \a flags.
     */
    explicit QHtmlAttributeMatch(const QString &name, const QString &value,
                                 QHtmlParser::MatchFlags flags = QHtmlParser::MatchFlags(QHtmlParser::MatchExactly));
    
    /*!
     * Returns the flags set for the attribute match.
     *
     * \sa testFlag()
     */
    QHtmlParser::MatchFlags flags() const;
    
    /*!
     * Sets the flags for the attribute match to \a flags.
     *
     * \sa setFlag()
     */
    void setFlags(QHtmlParser::MatchFlags flags);
    
    /*!
     * Sets or unsets the \a flag depending of the value of \a on (default is \c true).
     *
     * \sa setFlags()
     */
    void setFlag(QHtmlParser::MatchFlag flag, bool on = true);
    
    /*!
     * Returns \c true if \a flag is set.
     *
     * \sa flags()
     */
    bool testFlag(QHtmlParser::MatchFlag flag) const;

    /*!
     * Returns \c true if name(), value() and flags() of \a other are equal to those of this QHtmlAttributeMatch.
     */
    bool operator==(const QHtmlAttributeMatch &other) const;

    /*!
     * Returns \c true if either name(), value() or flags() of \a other are not equal to those of this QHtmlAttributeMatch.
     */
    bool operator!=(const QHtmlAttributeMatch &other) const;

private:
    QHtmlParser::MatchFlags m_flags;
};

class QHtmlElement;
class QHtmlElementPrivate;

/*!
 * Typedef for QList<QHtmlAttribute>.
 */
typedef QList<QHtmlAttribute> QHtmlAttributes;

/*!
 * Typedef for QList<QHtmlAttributeMatch>.
 */
typedef QList<QHtmlAttributeMatch> QHtmlAttributeMatches;

/*!
 * Typedef for QList<QHtmlElement>.
 */
typedef QList<QHtmlElement> QHtmlElementList;

/*!
 * Represents a HTML element/tag.
 */
class QHTMLPARSER_EXPORT QHtmlElement
{

public:
    /*!
     * Constructs a null QHtmlElement.
     */
    QHtmlElement();
    
    /*!
     * Contructs a copy of \a other.
     */
    QHtmlElement(const QHtmlElement &other);
    
    /*!
     * Destroys the QHtmlElement.
     */
    ~QHtmlElement();
    
    /*!
     * Returns the attributes of the element.
     */
    QHtmlAttributes attributes() const;
    
    /*!
     * Returns the value of the attribute with the specified \a name.
     *
     * If no attribute is found, an empty string is returned.
     */
    QString attribute(const QString &name) const;

    /*!
     * Returns the element's parent.
     */
    QHtmlElement parentElement() const;

    /*!
     * Returns the next sibling of the element.
     *
     * If the element has no next sibling, a null element is returned.
     */
    QHtmlElement nextSibling() const;

    /*!
     * Returns the previous sibling of the element.
     *
     * If the element has no previous sibling, a null element is returned.
     */
    QHtmlElement previousSibling() const;
    
    /*!
     * Returns all elements that are a direct child of the element.
     */
    QHtmlElementList childElements() const;
    
    /*!
     * Returns the first direct child of the element.
     *
     * If the element has no children, a null element is returned.
     */
    QHtmlElement firstChildElement() const;
    
    /*!
     * Returns the last direct child of the element.
     *
     * If the element has no children, a null element is returned.
     */
    QHtmlElement lastChildElement() const;
    
    /*!
     * Returns the \a nth direct child of the element.
     *
     * If the element has no children, a null element is returned.
     */
    QHtmlElement nthChildElement(int n) const;
    
    /*!
     * Returns the child of the element with 'id' attribute matching \a id.
     *
     * If no matching element is found, a null element is returned.
     */
    QHtmlElement elementById(const QString &id) const;
    
    /*!
     * Returns all children of the element with tagName() matching \a name.
     */
    QHtmlElementList elementsByTagName(const QString &name) const;
    
    /*!
     * \overload
     *
     * Returns all children of the element with tagName() matching \a name and attribute matching \a match.
     */
    QHtmlElementList elementsByTagName(const QString &name, const QHtmlAttributeMatch &match) const;
    
    /*!
     * \overload
     *
     * Returns all children of the element with tagName() matching \a name and attributes matching \a matches.
     */
    QHtmlElementList elementsByTagName(const QString &name, const QHtmlAttributeMatches &matches,
                                       QHtmlParser::MatchType matchType = QHtmlParser::MatchAll) const;
    
    /*!
     * Returns the first child with tagName() matching \a name.
     *
     * If no matching element is found, a null element is returned.
     */
    QHtmlElement firstElementByTagName(const QString &name) const;
    
    /*!
     * \overload
     *
     * Returns the first child with tagName() matching name and attribute matching match.
     */
    QHtmlElement firstElementByTagName(const QString &name, const QHtmlAttributeMatch &match) const;
    
    /*!
     * \overload
     *
     * Returns the first child with tagName() matching name and attributes matching matches.
     */
    QHtmlElement firstElementByTagName(const QString &name, const QHtmlAttributeMatches &matches,
                                       QHtmlParser::MatchType matchType = QHtmlParser::MatchAll) const;
    
    /*!
     * Returns the last child with tagName() matching name.
     *
     * If no matching element is found, a null element is returned.
     */
    QHtmlElement lastElementByTagName(const QString &name) const;
    
    /*!
     * \overload
     *
     * Returns the last child with tagName() matching \a name and attribute matching \a match.
     */
    QHtmlElement lastElementByTagName(const QString &name, const QHtmlAttributeMatch &match) const;
    
    /*!
     * \overload
     *
     * Returns the last child with tagName() matching \a name and attributes matching \a matches.
     */
    QHtmlElement lastElementByTagName(const QString &name, const QHtmlAttributeMatches &matches,
                                      QHtmlParser::MatchType matchType = QHtmlParser::MatchAll) const;
    
    /*!
     * Returns the \a nth child with tagName() matching \a name.
     *
     * If no matching element is found, a null element is returned.
     */
    QHtmlElement nthElementByTagName(int n, const QString &name) const;
    
    /*!
     * \overload
     *
     * Returns the \a nth child with tagName() matching \a name and attribute matching \a match.
     */
    QHtmlElement nthElementByTagName(int n, const QString &name, const QHtmlAttributeMatch &match) const;
    
    /*!
     * \overload
     *
     * Returns the nth child with tagName() matching \a name and attributes matching \a matches.
     */
    QHtmlElement nthElementByTagName(int n, const QString &name, const QHtmlAttributeMatches &matches,
                                     QHtmlParser::MatchType matchType = QHtmlParser::MatchAll) const;    
    
    /*!
     * Returns the tag name of the element.
     *
     * If the element is null, an empty string is returned.
     */
    QString tagName() const;
    
    /*!
     * Returns any text for the element, including any child elements if \a includeChildElements is \c true.
     *
     * If the element is null, an empty string is returned.
     */
    QString text(bool includeChildElements = false) const;
    
    /*!
     * Returns the HTML string of the element.
     *
     * If the element is null, an empty string is returned.
     */
    QString toString() const;
    
    /*!
     * Returns \c true if the element is null.
     *
     * An element is null if it does not represent a tag in a HTML document.
     */
    bool isNull() const;

    QHtmlElement& operator=(const QHtmlElement &other);
    
    /*!
     * Returns \c true if \a other represents the same tag in the same document as this QHtmlElement.
     */
    bool operator==(const QHtmlElement &other) const;
    
    /*!
     * Returns \c true if \a other does not represent the same tag in the same document as this QHtmlElement.
     */
    bool operator!=(const QHtmlElement &other) const;

private:
    QHtmlElementPrivate *d;
    
    friend class QHtmlDocument;
};

class QHtmlDocumentPrivate;
class QIODevice;

/*!
 * Represents a HTML document.
 *
 * The QHtmlDocument class is used for loading and parsing a HTML document.
 *
 * Example usage:
 *
 * \code
 * QFile file("document.html");
 * file.open(QFile::ReadOnly);
 * QHtmlDocument document;
 *
 * if (!document.setContent(&file)) {
 *     qDebug() << "Error:" << document.errorString();
 *     return;
 * }
 *
 * const QHtmlElement body = document.bodyElement();
 *
 * foreach (const QHtmlElement &element, body.childElements()) {
 *     // process element
 * }
 * \endcode
 */
class QHTMLPARSER_EXPORT QHtmlDocument
{

public:
    /*!
     * Constructs a null QHtmlDocument.
     */
    QHtmlDocument();
    
    /*!
     * Constructs a QHtmlDocument and sets the document content to \a content.
     *
     * \sa setContent()
     */
    explicit QHtmlDocument(const QString &content);
    
    /*!
     * Constructs a QHtmlDocument and sets the document content to \a content.
     *
     * \sa setContent()
     */
    explicit QHtmlDocument(const QByteArray &content);
    
    /*!
     * Constructs a QHtmlDocument and sets the document content to the data contained in \a device.
     *
     * The device should be open and ready for reading the entire document.
     *
     * \sa setContent()
     */
    explicit QHtmlDocument(QIODevice *device);
    
    /*!
     * Destroys the QHtmlDocument.
     *
     * \warning Any instances of QHtmlElement associated with this document 
     * will become invalid.
     */
    ~QHtmlDocument();
    
    /*!
     * Sets the document content to \a content.
     *
     * Returns true if the content can be parsed, otherwise false.
     *
     * \warning Any instances of QHtmlElement associated with this document 
     * will become invalid.
     */
    bool setContent(const QString &content);
    
    /*!
     * \overload
     */
    bool setContent(const QByteArray &content);
    
    /*!
     * \overload
     *
     * Sets the document content to the data contained in \a device.
     *
     * The device should be open and ready for reading the entire document.
     */
    bool setContent(QIODevice *device);
    
    /*!
     * Returns the root element of the document.
     *
     * If the document is null, a null element is returned.
     */
    QHtmlElement documentElement() const;
    
    /*!
     * Returns the html element of the document (the element with tag name 'html').
     *
     * If the document is null, a null element is returned.
     */
    QHtmlElement htmlElement() const;
    
    /*!
     * Returns the head element of the document (the element with tag name 'head').
     *
     * If the document is null, a null element is returned.
     */
    QHtmlElement headElement() const;
    
    /*!
     * Returns the body element of the document (the element with tag name 'body').
     *
     * If the document is null, a null element is returned.
     */
    QHtmlElement bodyElement() const;
    
    /*!
     * Returns the HTML string of the document.
     *
     * If the document is null, an empty string is returned.
     */
    QString toString() const;
    
    /*!
     * Returns \c true if an error occurred when parsing the document.
     */
    bool hasError() const;
    
    /*!
     * Returns a description of any error that occurred when parsing the document.
     *
     * If no error occurred, an empty string is returned.
     */
    QString errorString() const;
    
    /*!
     * Returns \c true if the document is null.
     *
     * The document is null if no content has been set.
     */
    bool isNull() const;

private:
    QHtmlDocumentPrivate *d;
    Q_DISABLE_COPY(QHtmlDocument)
};

#endif // QHTMLPARSER_H
