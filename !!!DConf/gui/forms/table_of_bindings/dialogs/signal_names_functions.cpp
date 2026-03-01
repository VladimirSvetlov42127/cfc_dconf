
#include "signal_names_functions.h"

bool hasTag(const QString & initName)
{
	if(initName.lastIndexOf('<') >= 0 || initName.lastIndexOf('>') >= 0)
		return true;
	return false;
}

void removeTagFromName(DcSignal *signal, const QString& nameToRemove) {
	if (nameToRemove.isEmpty() || signal == nullptr)
		return;

	QString name = signal->name();
	if (!name.contains(nameToRemove))
		return;

	int16_t index = name.indexOf(nameToRemove);	// Ориентир от которого ищем тэги
	int16_t indexOpen = name.lastIndexOf('<', index);	// Должен быть перед 
	int16_t indexClose = name.indexOf('>', index);	// и после
	QString newName;
	
	if (indexOpen >= 0 && indexClose >= 0) {	// Если есть теги спереди и сзади, удаляем их
		newName = name.remove(indexOpen, 1);
		newName = newName.remove(name.indexOf('>', index), 1);	// Рассчитываем заново, т.к. удалился символ
		newName = newName.remove(nameToRemove);
	}
	else {
		newName = name.remove(nameToRemove);	// Или только имя
	}

	signal->updateName(newName);
}

QString addInputTag(const QString & tag, const QString &initName)
{
	if (tag.isEmpty() || initName.isEmpty())
		return QString();

	return ('<' + tag + '>' + initName);
}

QString addOutputTag(const QString & tag, const QString &initName)
{
	if (tag.isEmpty() || initName.isEmpty())
		return QString();

	return (initName + '<' + tag + '>');
}

QString removeAlgInput(const QString & algInputName, const QString & initName)
{
	if (algInputName.isEmpty() || initName.isEmpty())
		return QString();

	if (!initName.contains(algInputName))
		return QString();

	QString newName;
	if (hasTag(initName)) {	// Если имя было составное
		newName = initName;
		int indexName = newName.indexOf(algInputName);
		newName = newName.remove(initName.lastIndexOf('>', indexName), 1); // Удаляем с конца, чтобы было валидно значение indexName
		newName = newName.remove(initName.indexOf('<'), 1);
		newName = newName.remove(algInputName);
	}
	else {	// Если в имени было только имя входа
		newName = VIRTUAL_DEFAULT_NAME;
	}

	return newName;
}
