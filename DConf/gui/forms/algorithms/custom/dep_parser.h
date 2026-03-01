#ifndef __DEP_PARSER_H__
#define __DEP_PARSER_H__

//===================================================================================================================================================
//	Подключение стандартных библиотек
//===================================================================================================================================================
#include <cstdint>

//===================================================================================================================================================
//	Подключение библиотек QT
//===================================================================================================================================================
#include <QObject>
#include <QList>
#include <QString>
#include <QDomDocument>
#include <QPointF>
#include <QSizeF>

//===================================================================================================================================================
//	Подключение модулей проекта
//===================================================================================================================================================
#include <gui/forms/algorithms/custom/Editor/editor_node.h>
#include <gui/forms/algorithms/custom/Editor/editor_link.h>


//===================================================================================================================================================
//	Описание класса
//===================================================================================================================================================
class DepCfcParser : public QObject
{
	Q_OBJECT

public:
	//===============================================================================================================================================
	//	Конструктор и деструктор класса
	//===============================================================================================================================================
	DepCfcParser();
    DepCfcParser(const QList<EditorNode*>& nodes, const QList<EditorLink*>& links);
	~DepCfcParser();

    //	Правило пяти
    DepCfcParser(const DepCfcParser& other) = delete;
    DepCfcParser(DepCfcParser&& other) = delete;
    DepCfcParser& operator=(const DepCfcParser& other) = delete;
    DepCfcParser& operator=(DepCfcParser&& other) = delete;

	//===============================================================================================================================================
	//	Методы работы со свойствами класса
	//===============================================================================================================================================
	uint8_t version() const { return _version; }
	QString title() { return _title; }
	QStringList errors() { return _error_list; }
	QList<EditorNode*> editorNodes() { return _nodes; }
	QList<EditorLink*> editorLinks() { return _links; }
    void setTile(const QString& title) { _title = title; }
    void setCanvas(int width, int height);

	//===============================================================================================================================================
	//	Открытые методы класса
	//===============================================================================================================================================
	void init();
	bool loadFile(const QString& file_name);
	bool saveFile(const QString& file_name);
	bool validate();
    void setData(const QList<EditorNode*>& nodes, const QList<EditorLink*>& links);
	void setTitle(const QString& title) { _title = title; }
    QList<uint16_t> cfcSignals() const;

signals:
	//===============================================================================================================================================
	//	Сигналы парсера
	//===============================================================================================================================================
	void errorToLog(const QString& message);
	void infoToLog(const QString& message);

private:
	//===============================================================================================================================================
	//	Методы обработки XML
	//===============================================================================================================================================
    EditorNode* newEditorNode(const QString& name, const QString& id, const QSizeF& node_size = QSizeF());
    EditorNode* nodeFromXML(QDomNode xml);
    EditorLink* linkFromXML(QDomNode xml, QList<EditorNode*> nodes);
    void linkNodes(const QList<EditorNode*>& nodes, const QList<EditorLink*>& links);
	bool deserialize(QDomDocument xml);
	void nodeToXML(EditorNode* node, QDomDocument& xml, QDomElement& xml_node);
	void linkToXML(EditorLink* link, QDomDocument& xml, QDomElement& xml_link);
	QDomDocument serialize();

	//===============================================================================================================================================
	//	Свойства класса
	//===============================================================================================================================================
	uint8_t _version;
	QString _title;
	QStringList _error_list;
	QList<EditorNode*> _nodes;
	QList<EditorLink*> _links;
	int _width;
	int _height;
};

#endif	//	__DEP_PARSER_H__
