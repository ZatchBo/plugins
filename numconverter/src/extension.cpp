#include <QDebug>
#include <QClipboard>
#include <QPointer>
#include <stdexcept>
#include "util/standarditem.h"
#include "xdg/iconlookup.h"
#include "configwidget.h"
#include "extension.h"
using namespace Core;
using namespace std;


namespace {
    const QStringList numberBases = {
        "b",
        "o",
        "d",
        "h",
    };
    const QStringList baseName = {
        "bin",
        "oct",
        "dec",
        "hex",
    };
}


class NumConverter::Private
{
public:
    QPointer<ConfigWidget> widget;
};


/** ***************************************************************************/
NumConverter::Extension::Extension()
    : Core::Extension("org.albert.extension.numconverter"), // Must match the id in metadata
      Core::QueryHandler(Core::Plugin::id()),
      d(new Private) {

    registerQueryHandler(this);
}



/** ***************************************************************************/
NumConverter::Extension::~Extension() {

}



/** ***************************************************************************/
QWidget *NumConverter::Extension::widget(QWidget *parent) {
    if (d->widget.isNull()) {
        d->widget = new ConfigWidget(parent);
    }
    return d->widget;
}

/** ***************************************************************************/
QStringList NumConverter::Extension::triggers() const {
    return {
        "h ",
        "o ",
        "d ",
        "b ",
    };
}

/** ***************************************************************************/
void NumConverter::Extension::setupSession() {

}



/** ***************************************************************************/
void NumConverter::Extension::teardownSession() {

}



/** ***************************************************************************/
void NumConverter::Extension::handleQuery(Core::Query *) const {

    auto buildItem = [](int base, QString str_num){

        stringstream converted_numbers[4];

        switch ( numberBases[base] ) {
            case "b": {
                auto int_num = stoul(str_num, NULL, 2);
                break;
            }
            case "o": {
                auto int_num = stoul(str_num, NULL, 8);
                break;
            }
            case "d": {
                auto int_num = stoul(str_num, NULL, 10);
                break;
            }
            case "h": {
                auto int_num = stoul(str_num, NULL, 16);
                break;
            }
        }

        string bin_string;
        if ( numberBases[base] != "b" ) {
            // Make the binary string
            int bits = ( (sizeof(int_num)*8)-1 );
            for (int i=bits; i>=0; i--) {
                if ( (int_num >> i) & 1) == 1 ) {
                    bin_string.append("1");
                }
                else {
                    bin_string.append("0");
                }
            }
        }
        else {
            bin_string = str_num;
        }


        converted_numbers[0].write(bin_string);
        converted_numbers[1] << oct << int_num;
        converted_numbers[2] << dec << int_num;
        converted_numbers[3] << hex << int_num;

        auto item = make_shared<StandardItem>(numberBases[base]);
        item->setSubtext(QString("%1 as %2").arg(string, baseName[base]));
        item->setText(converted_numbers[base]);
        item->setIconPath(":hash");
        item->setCompletion(QString("%1 %2").arg(numberBases[base].toLower(), string));
        item->addAction(make_shared<ClipAction>("Copy hash value to clipboard",
                            QString(converted_numbers[base])));
        return item;
    };

    auto requestedBase = find(numberBases.begin(), numberBases.end(),
                            query->trigger().trimmed().toLower());
    if ( requestedBase != numberBases.end()) {
        int base = static_cast<int>(distance(numberBases.begin(), requestedBase));
        query->addMatch(buildItem(base, query->string()));
    }
}
