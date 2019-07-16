#include "encodingdetectdialog.h"
#include "ui_encodingdetectdialog.h"

#include <QPushButton>
#include <QScrollBar>
#include <QTextCodec>
#include <QTextStream>

#include <KCharsets>

using namespace SubtitleComposer;

#define INVALID_PARENT_ROW quintptr(-1)

namespace SubtitleComposer {
class TreeModel : public QAbstractItemModel
{
	Q_OBJECT
	friend class EncodingDetectDialog;

public:
	explicit TreeModel(QObject *parent = nullptr);
	~TreeModel();

	QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &index) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override { Q_UNUSED(parent); return 1; }
	const QString & encoding(const QModelIndex &index) const;

private:
	void setupModelData();

	QList<QStringList> m_encodings;
	QStringList m_detectedName;
	QStringList m_detected;
};
}

EncodingDetectDialog::EncodingDetectDialog(const QByteArray &text, QWidget *parent)
	: QDialog(parent),
	  ui(new Ui::EncodingDetectDialog),
	  m_text(text)
{
	ui->setupUi(this);
	TreeModel *model = new TreeModel(this);
	ui->encoding->setModel(model);

	ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(true);
	connect(ui->encoding->selectionModel(), &QItemSelectionModel::selectionChanged, [&, model](const QItemSelection &sel, const QItemSelection &){
		const bool hasSelection = !sel.isEmpty() && !sel.first().isEmpty();
		ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(hasSelection);

		if(hasSelection) {
			const QModelIndex first = sel.first().indexes().first();
			m_selectedEncoding = model->encoding(first);
			QTextCodec *codec = KCharsets::charsets()->codecForName(m_selectedEncoding);
			QTextStream textStream(m_text);
			textStream.setCodec(codec);
			const int val = ui->preview->verticalScrollBar()->value();
			ui->preview->setPlainText(textStream.readAll());
			ui->preview->verticalScrollBar()->setValue(val);
		} else {
			ui->preview->clear();
			m_selectedEncoding.clear();
		}
	});
}

EncodingDetectDialog::~EncodingDetectDialog()
{
	delete ui;
}

void
EncodingDetectDialog::addEncoding(const QString &name, int confidence)
{
	TreeModel *model = reinterpret_cast<TreeModel *>(ui->encoding->model());
	model->m_detectedName.append(name);
	model->m_detected.append(i18nc("Text encoding detected with n% confidence.", "%1 (%2% confidence)",
		name.toUpper(), confidence));

	if(ui->encoding->selectionModel()->selection().isEmpty()) {
		const QModelIndex &detected = model->index(0, 0);
		ui->encoding->expand(detected);
		const QModelIndex &selected = model->index(0, 0, detected);
		ui->encoding->selectionModel()->select(selected, QItemSelectionModel::ClearAndSelect);
	}
}

/// Tree Model
TreeModel::TreeModel(QObject *parent)
	: QAbstractItemModel(parent)
{
	m_encodings = KCharsets::charsets()->encodingsByScript();
	for(QStringList &encList : m_encodings) {
		bool first = true;
		for(QString &enc: encList) {
			if(first)
				first = false;
			else
				enc = enc.toUpper();
		}
	}
}

TreeModel::~TreeModel() {}

const QString &
TreeModel::encoding(const QModelIndex &index) const
{
	int p = index.parent().row();
	const QStringList &parentList = p == 0 ? m_detectedName : m_encodings.at(p - 1);
	return parentList.at(p == 0 ? index.row() : index.row() + 1);
}

Qt::ItemFlags
TreeModel::flags(const QModelIndex &index) const
{
	if(!index.isValid())
        return Qt::NoItemFlags;
	if(index.internalId() != INVALID_PARENT_ROW)
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
	return Qt::ItemIsEnabled;
}

QVariant
TreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if(orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
        return QStringLiteral("Encoding");

	return QVariant();
}

QModelIndex
TreeModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_ASSERT(column == 0);

	if(!parent.isValid()) {
		if(row <= m_encodings.size())
			return createIndex(row, 0, INVALID_PARENT_ROW);
		return QModelIndex();
	}

	Q_ASSERT(parent.internalId() == INVALID_PARENT_ROW);

	if(row < rowCount(parent))
		return createIndex(row, 0, parent.row());

	return QModelIndex();
}

QModelIndex
TreeModel::parent(const QModelIndex &index) const
{
	if(!index.isValid())
		return QModelIndex();

	if(index.internalId() == INVALID_PARENT_ROW)
		return QModelIndex();

	return createIndex(index.internalId(), 0, INVALID_PARENT_ROW);
}

int
TreeModel::rowCount(const QModelIndex &parent) const
{
	if(parent.column() > 0)
		return 0;

	if(!parent.isValid())
		return m_encodings.size() + 1;

	if(parent.internalId() != INVALID_PARENT_ROW)
		return 0;

	return parent.row() == 0 ? m_detected.size() : m_encodings.at(parent.row() - 1).size() - 1;
}

QVariant
TreeModel::data(const QModelIndex &index, int role) const
{
	if(!index.isValid() || role != Qt::DisplayRole)
		return QVariant();

	const QModelIndex &parentIndex = index.parent();
	if(!parentIndex.isValid())
		return index.row() == 0 ? i18n("Detected") : m_encodings.at(index.row() - 1).at(0);

	const QStringList &parentList = parentIndex.row() == 0 ? m_detected : m_encodings.at(parentIndex.row() - 1);
	return parentList.at(parentIndex.row() == 0 ? index.row() : index.row() + 1);
}

#include "encodingdetectdialog.moc"
