#pragma once
#include <qstring.h>

class DcProjectDescription {

public:
	DcProjectDescription();
	DcProjectDescription(const QString &name, const QString &object, const QString &author, const QString &desc,
		const QString &timeCreate, const QString &timeUpdate);
	~DcProjectDescription();

private:
	QString m_name;
	QString m_object;
	QString m_author;
	QString m_description;
	QString m_timeCreate;
	QString m_timeUpdate;
	uint32_t m_version;
	bool m_isNeedUpdate;

public:
	bool init(const QString& path);
	bool save(const QString& path);
	void update();
    // bool isNeedUpdate();
	void clear(void);

	void setName(const QString &name);
	void setObject(const QString &obj);
	void setAuthor(const QString &author);
	void setDesc(const QString &desc);
	void setCreateTime(const QString &timeCreate);
	void setUpdateTime(const QString &timeUpdate);
	void setVersion(uint32_t ver);

	QString name();
	QString object();
	QString author();
	QString desc();
	QString timeCreate();
	QString timeUpdate();
	uint32_t version() const { return m_version; }

private:
	QString toJSon();
	bool fromJson(QString &jsonstr);
};
