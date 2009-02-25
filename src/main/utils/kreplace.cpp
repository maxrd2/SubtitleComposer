/*
    Copyright (C) 2001, S.R.Haque <srhaque@iee.org>.
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "kreplace.h"
#include "kfind_p.h"

#include <QtGui/QLabel>
#include <KApplication>
#include <kdebug.h>

#include <klocale.h>
#include <KMessageBox>
#include "kreplacedialog.h"
#include <QtCore/QRegExp>

//#define DEBUG_REPLACE
#define INDEX_NOMATCH -1

class KReplaceNextDialog : public KDialog
{
public:
    explicit KReplaceNextDialog( QWidget *parent );
    void setLabel( const QString& pattern, const QString& replacement );
private:
    QLabel* m_mainLabel;
};

KReplaceNextDialog::KReplaceNextDialog(QWidget *parent) :
    KDialog(parent)
{
    setModal( false );
    setCaption( i18n("Replace") );
    setButtons( User3 | User2 | User1 | Close );
    setButtonGuiItem( User1, KGuiItem(i18n("&All")) );
    setButtonGuiItem( User2, KGuiItem(i18n("&Skip")) );
    setButtonGuiItem( User3, KGuiItem(i18n("Replace")) );
    setDefaultButton( User3 );
    showButtonSeparator( false );

    m_mainLabel = new QLabel( this );
    setMainWidget( m_mainLabel );
}

void KReplaceNextDialog::setLabel( const QString& pattern, const QString& replacement )
{
    m_mainLabel->setText( i18n("Replace '%1' with '%2'?", pattern, replacement) );
}

////

class KReplacePrivate
{
public:
    KReplacePrivate(KReplace *q, const QString& replacement)
        : q(q)
        , m_replacement( replacement )
        , m_replacements( 0 )
    {}

    KReplaceNextDialog* dialog();
    void doReplace();
    //static int replace( QString &text, const QString &replacement, int index, long options, int length );
    static int replace(QString& text, const QString& replacement, QRegExp* regExp, int index, long options, int length);

    void _k_slotSkip();
    void _k_slotReplace();
    void _k_slotReplaceAll();

    KReplace *q;
    QString m_replacement;
    unsigned m_replacements;
};


////

KReplace::KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent) :
    KFind( pattern, options, parent ),
    d( new KReplacePrivate(this, replacement) )
{
}

KReplace::KReplace(const QString &pattern, const QString &replacement, long options, QWidget *parent, QWidget *dlg) :
    KFind( pattern, options, parent, dlg ),
    d( new KReplacePrivate(this, replacement) )
{
}

KReplace::~KReplace()
{
    delete d;
}

int KReplace::numReplacements() const
{
    return d->m_replacements;
}

KDialog* KReplace::replaceNextDialog( bool create )
{
    if ( KFind::d->dialog || create )
        return d->dialog();
    return 0L;
}

KReplaceNextDialog* KReplacePrivate::dialog()
{
    if ( !q->KFind::d->dialog )
    {
        q->KFind::d->dialog = new KReplaceNextDialog( q->parentWidget() );
        q->connect( q->KFind::d->dialog, SIGNAL( user1Clicked() ), q, SLOT( _k_slotReplaceAll() ) );
        q->connect( q->KFind::d->dialog, SIGNAL( user2Clicked() ), q, SLOT( _k_slotSkip() ) );
        q->connect( q->KFind::d->dialog, SIGNAL( user3Clicked() ), q, SLOT( _k_slotReplace() ) );
        q->connect( q->KFind::d->dialog, SIGNAL( finished() ), q, SLOT( _k_slotDialogClosed() ) );
    }
    return static_cast<KReplaceNextDialog *>(q->KFind::d->dialog);
}

void KReplace::displayFinalDialog() const
{
    if ( !d->m_replacements )
        KMessageBox::information(parentWidget(), i18n("No text was replaced."));
    else
        KMessageBox::information(parentWidget(), i18np("1 replacement done.", "%1 replacements done.", d->m_replacements ) );
}

KFind::Result KReplace::replace()
{
#ifdef DEBUG_REPLACE
    kDebug() << "d->index=" << KFind::d->index;
#endif
    if ( KFind::d->index == INDEX_NOMATCH && KFind::d->lastResult == Match )
    {
        KFind::d->lastResult = NoMatch;
        return NoMatch;
    }

    do // this loop is only because validateMatch can fail
    {
#ifdef DEBUG_REPLACE
        kDebug() << "beginning of loop: KFind::d->index=" << KFind::d->index;
#endif
        // Find the next match.
        if ( KFind::d->options & KFind::RegularExpression )
            KFind::d->index = KFind::find(KFind::d->text, *KFind::d->regExp, KFind::d->index, KFind::d->options, &KFind::d->matchedLength);
        else
            KFind::d->index = KFind::find(KFind::d->text, KFind::d->pattern, KFind::d->index, KFind::d->options, &KFind::d->matchedLength);

#ifdef DEBUG_REPLACE
        kDebug() << "KFind::find returned KFind::d->index=" << KFind::d->index;
#endif
        if ( KFind::d->index != -1 )
        {
            // Flexibility: the app can add more rules to validate a possible match
            if ( validateMatch( KFind::d->text, KFind::d->index, KFind::d->matchedLength ) )
            {
                if ( KFind::d->options & KReplaceDialog::PromptOnReplace )
                {
#ifdef DEBUG_REPLACE
                    kDebug() << "PromptOnReplace";
#endif
                    // Display accurate initial string and replacement string, they can vary
                    QString matchedText (KFind::d->text.mid( KFind::d->index, KFind::d->matchedLength ));
                    QString rep (matchedText);
                    d->KReplacePrivate::replace(rep, d->m_replacement, KFind::d->regExp, 0, KFind::d->options, KFind::d->matchedLength);
                    d->dialog()->setLabel( matchedText, rep );
                    d->dialog()->show();

                    // Tell the world about the match we found, in case someone wants to
                    // highlight it.
                    emit highlight(KFind::d->text, KFind::d->index, KFind::d->matchedLength);

                    KFind::d->lastResult = Match;
                    return Match;
                }
                else
                {
                    d->doReplace(); // this moves on too
                }
            }
            else
            {
                // not validated -> move on
                if (KFind::d->options & KFind::FindBackwards)
                    KFind::d->index--;
                else
                    KFind::d->index++;
            }
        } else
            KFind::d->index = INDEX_NOMATCH; // will exit the loop
    }
    while (KFind::d->index != INDEX_NOMATCH);

    KFind::d->lastResult = NoMatch;
    return NoMatch;
}

int KReplace::replace(QString &text, const QString &pattern, const QString &replacement, int index, long options, int *replacedLength)
{
    if ( (options & KFind::RegularExpression) && (options & KReplaceDialog::BackReference) )
    {
        QRegExp regExp( pattern, options & KFind::CaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive );
        return KReplace::replace(text, regExp, replacement, index, options, replacedLength);
    }
    else
    {
        int matchedLength;

        index = KFind::find(text, pattern, index, options, &matchedLength);
        if (index != -1)
        {
            *replacedLength = KReplacePrivate::replace(text, replacement, 0, index, options, matchedLength);
            if (options & KFind::FindBackwards)
                index--;
            else
                index += *replacedLength;
        }
        return index;
    }
}

int KReplace::replace(QString &text, const QRegExp &pattern, const QString &replacement, int index, long options, int *replacedLength)
{
    int matchedLength;

    index = KFind::find(text, pattern, index, options, &matchedLength);
    if (index != -1)
    {
        QRegExp regExp( pattern );
        *replacedLength = KReplacePrivate::replace(text, replacement, &regExp, index, options, matchedLength);
        if (options & KFind::FindBackwards)
            index--;
        else
            index += *replacedLength;
    }
    return index;
}

// int KReplacePrivate::replace(QString &text, const QString &replacement, int index, long options, int length)
// {
//     QString rep (replacement);
//     // Backreferences: replace \0 with the right portion of 'text'
//     if ( options & KReplaceDialog::BackReference )
//         rep.replace( "\\0", text.mid( index, length ) );
//     // Then replace rep into the text
//     text.replace(index, length, rep);
//     return rep.length();
// }

int KReplacePrivate::replace(QString& text, const QString& replacement, QRegExp* regExp, int index, long options, int length)
{
    // Backreferences: replace \N with the right portion of 'text'
    if ( regExp && (options & KReplaceDialog::BackReference) )
    {
        QString mutableReplacement( replacement );

        // NOTE we have to build the replacement string from the regExp, the
        // matched text and the replacement template.
        // We can't do QString::replace( *regExp, replacement ) on the matched
        // text because that ignores the \0 and we can't replace it ourselves
        // before nor after that call as both the matched text and the matched
        // subexpressions can contain text that may look as backreferences but
        // which should be not be considered as such.
        // For the same reason we can't just do a QString::replace( "\\N",
        // regExp.cap( N ) ) on all subexpressions found.
        // There's no choice but to iterate the replacement template, replacing
        // the back references with the corresponding captures as we find them.

        if ( regExp->indexIn( text.mid( index, length ) /* the matched text */ ) != -1 ) // set up the captures
        {
            int capNumber, maxCapNumber = qMin( 9, regExp->numCaptures() );

            int idx = 0;
            while ( idx < mutableReplacement.length() )
            {
                if ( mutableReplacement.at( idx ) == '\\' )
                {
                    idx++;
                    if ( idx >= mutableReplacement.length() )
                        break;

                    QChar chr = mutableReplacement.at( idx );
                    capNumber = chr.digitValue();
                    if ( capNumber >= 0 && capNumber <= maxCapNumber )
                    {
                        QString cap( regExp->cap( capNumber ) );
                        mutableReplacement.replace( idx-1, 2, cap );
                        idx += cap.length() - 1;
                    }
                    else if ( chr == '\\' )
                    {
                        mutableReplacement.replace( idx-1, 2, '\\' );
                    }
                    else
                        idx++;
                }
                else
                    idx++;
            }
        }

        text.replace( index, length, mutableReplacement );
        return mutableReplacement.length();
    }
    else
    {
        text.replace( index, length, replacement );
        return replacement.length();
    }
}

void KReplacePrivate::_k_slotReplaceAll()
{
    doReplace();
    q->KFind::d->options &= ~KReplaceDialog::PromptOnReplace;
    emit q->optionsChanged();
    emit q->findNext();
}

void KReplacePrivate::_k_slotSkip()
{
    if (q->KFind::d->options & KFind::FindBackwards)
        q->KFind::d->index--;
    else
        q->KFind::d->index++;
    if ( q->KFind::d->dialogClosed ) {
        delete q->KFind::d->dialog; // hide it again
        q->KFind::d->dialog = 0L;
    } else
        emit q->findNext();
}

void KReplacePrivate::_k_slotReplace()
{
    doReplace();
    if ( q->KFind::d->dialogClosed ) {
        delete q->KFind::d->dialog; // hide it again
        q->KFind::d->dialog = 0L;
    } else
        emit q->findNext();
}

void KReplacePrivate::doReplace()
{
    int replacedLength = replace(q->KFind::d->text, m_replacement, q->KFind::d->regExp, q->KFind::d->index, q->KFind::d->options, q->KFind::d->matchedLength);

    // Tell the world about the replacement we made, in case someone wants to
    // highlight it.
    emit q->replace(q->KFind::d->text, q->KFind::d->index, replacedLength, q->KFind::d->matchedLength);
#ifdef DEBUG_REPLACE
    kDebug() << "after replace() signal: KFind::d->index=" << q->KFind::d->index << " replacedLength=" << replacedLength;
#endif
    m_replacements++;
    if (q->KFind::d->options & KFind::FindBackwards)
        q->KFind::d->index--;
    else {
        q->KFind::d->index += replacedLength;
        // when replacing the empty pattern, move on. See also kjs/regexp.cpp for how this should be done for regexps.
        if ( q->KFind::d->pattern.isEmpty() )
            ++(q->KFind::d->index);
    }
#ifdef DEBUG_REPLACE
    kDebug() << "after adjustement: KFind::d->index=" << q->KFind::d->index;
#endif
}

void KReplace::resetCounts()
{
    KFind::resetCounts();
    d->m_replacements = 0;
}

bool KReplace::shouldRestart( bool forceAsking, bool showNumMatches ) const
{
    // Only ask if we did a "find from cursor", otherwise it's pointless.
    // ... Or if the prompt-on-replace option was set.
    // Well, unless the user can modify the document during a search operation,
    // hence the force boolean.
    if ( !forceAsking && (KFind::d->options & KFind::FromCursor) == 0
         && (KFind::d->options & KReplaceDialog::PromptOnReplace) == 0 )
    {
        displayFinalDialog();
        return false;
    }
    QString message;
    if ( showNumMatches )
    {
        if ( !d->m_replacements )
            message = i18n("No text was replaced.");
        else
            message = i18np("1 replacement done.", "%1 replacements done.", d->m_replacements );
    }
    else
    {
        if ( KFind::d->options & KFind::FindBackwards )
            message = i18n( "Beginning of document reached." );
        else
            message = i18n( "End of document reached." );
    }

    message += '\n';
    // Hope this word puzzle is ok, it's a different sentence
    message +=
        ( KFind::d->options & KFind::FindBackwards ) ?
        i18n("Do you want to restart search from the end?")
        : i18n("Do you want to restart search at the beginning?");

    int ret = KMessageBox::questionYesNo( parentWidget(), message, QString(), KGuiItem(i18n("Restart")), KGuiItem(i18n("Stop")) );
    return( ret == KMessageBox::Yes );
}

void KReplace::closeReplaceNextDialog()
{
    closeFindNextDialog();
}

#include "kreplace.moc"
