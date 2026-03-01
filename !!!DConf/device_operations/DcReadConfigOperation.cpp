#include "DcReadConfigOperation.h"

#include <qdir.h>
#include <qtemporarydir.h>
#include <qdebug.h>

#include <dpc/sybus/channel/Channel.h>
#include <dpc/sybus/utils.h>
#include <db/dc_db_manager.h>
#include <utils/qzipreader_p.h>

#include <file_managers/IFileManager.h>

using namespace Dpc::Sybus;

namespace {
	enum OperationSteps {
		ReadParamStep = 0,
		ReadFilesStep,
		ReadArchiveStep,
		UnpackArchiveStep,

		TotalStepsCount
	};

	bool copyCfcAlgs(DcController *pmodelctrl, DcController *punitctrl) {
		uint32_t localsize = pmodelctrl->algs_cfc()->size();
		for (uint32_t i = 0; i < localsize; i++) {
			DcAlgCfc *pelem = pmodelctrl->algs_cfc()->get(i);
			pelem->ios()->removeAll();
		}

		pmodelctrl->algs_cfc();
		pmodelctrl->algs_cfc()->removeAll();

		if (!punitctrl)
			return true;

		uint32_t size = punitctrl->algs_cfc()->size();
		if (size < 1)
			return true;

		for (uint32_t i = 0; i < size; i++) {
			DcAlgCfc *pelem = punitctrl->algs_cfc()->get(i);
            DcAlgCfc *pnewelem = new DcAlgCfc(pmodelctrl->index(), pelem->index(), pelem->position(), pelem->name(), pelem->properties());
			pmodelctrl->algs_cfc()->add(pnewelem, false);
			uint32_t iossize = pelem->ios()->size();
			for (uint32_t j = 0; j < iossize; j++) {
				DcAlgIOCfc *pcfcio = pelem->ios()->get(j);
				DcAlgIOCfc *pnewcfcio = new DcAlgIOCfc(pmodelctrl->index(), pcfcio->index(), pcfcio->alg(), pcfcio->pin(), pcfcio->direction(), pcfcio->name());
				pnewelem->ios()->add(pnewcfcio, false);
			}
		}
		return true;
	}

    bool copySignalsNames(DcController *pmodelctrl, DcController *punitctrl) {
		if (!punitctrl)
			return true;

		auto makeKey = [](DcSignal* s) { return QString("%1-%2-%3-%4").arg(s->type()).arg(s->direction()).arg(s->subtype()).arg(s->internalId()); };
		QHash<QString, DcSignal*> unitSignals;
		for (auto s: punitctrl->getSignalList()) {
			unitSignals[makeKey(s)] = s;
		}

		for (auto s: pmodelctrl->getSignalList()) {
			DcSignal *unitSignal = unitSignals[makeKey(s)];
			if (unitSignal && s->name() != unitSignal->name())
				s->updateName(unitSignal->name());
		}

		return true;
	}

	bool copyMatrixAlgs(DcController *pmodelctrl, DcController *punitctrl) {
		pmodelctrl->matrix_alg()->removeAll();
		if (!punitctrl)
			return true;

		uint32_t size = punitctrl->matrix_alg()->size();
		for (uint32_t i = 0; i < size; i++) {
			DcMatrixElementAlg *pelem = punitctrl->matrix_alg()->get(i);
            DcMatrixElementAlg *pnewelem = new DcMatrixElementAlg(pmodelctrl->index(), pelem->src(), pelem->dst(), "");
			pmodelctrl->matrix_alg()->add(pnewelem, false);
		}
		return true;
	}

	bool copyMatrixAlgsCfc(DcController *pmodelctrl, DcController *punitctrl) {
		pmodelctrl->matrix_cfc()->removeAll();
		if (!punitctrl)
			return true;

		uint32_t size = punitctrl->matrix_cfc()->size();
		for (uint32_t i = 0; i < size; i++) {
			DcMatrixElementAlgCfc *pelem = punitctrl->matrix_cfc()->get(i);
            DcMatrixElementAlgCfc *pnewelem = new DcMatrixElementAlgCfc(pmodelctrl->index(), pelem->src(), pelem->dst(), "");
            pmodelctrl->matrix_cfc()->add(pnewelem, false);
		}
		return true;
	}

	bool copyMatrixSignals(DcController *pmodelctrl, DcController *punitctrl) {
		pmodelctrl->matrix_signals()->removeAll();
		if (!punitctrl)
			return true;

		uint32_t size = punitctrl->matrix_signals()->size();
		for (uint32_t i = 0; i < size; i++) {
			DcMatrixElementSignal *pelem = punitctrl->matrix_signals()->get(i);
            DcMatrixElementSignal *pnewelem = new DcMatrixElementSignal(pmodelctrl->index(), pelem->src(), pelem->dst(), "");
			pmodelctrl->matrix_signals()->add(pnewelem, false);
		}
		return true;
	}

    void configureMatrixAlgs(DcController *device) {
        // Костыль чтобы воставновить привязки внутрених алгоритмов
        device->matrix_alg()->removeAll();
        for (uint32_t i = 0; i < device->algs_internal()->size(); i++) {
            auto alg = device->algs_internal()->get(i);
            uint32_t addr = alg->property("addr").toUInt(nullptr, 16);
            if (!addr)
                continue;

            auto makeAlgMatrix = [=](uint16_t addr, int direction) {
                auto algBindParam = dynamic_cast<DcParamCfg_v2*>(device->getParam(addr, 0));
                if (!algBindParam)
                    return;

                for (size_t j = 0; j < algBindParam->getSubProfileCount(); j++) {
                    auto p = device->getParam(addr, j);
                    auto sigInternalId = p->value().toUInt();
                    if (sigInternalId == 0xFFFF)
                        continue;

                    DcAlgIOInternal *algPin = nullptr;
                    for (size_t k = 0; k < alg->ios()->size(); k++) {
                        auto ios = alg->ios()->get(k);
                        if (ios->pin() == j && ios->direction() == direction) {
                            algPin = ios;
                            break;
                        }
                    }
                    if (!algPin)
                        continue;

                    DcSignal* signal = device->getSignal(sigInternalId, DEF_SIG_TYPE_DISCRETE);
                    if (!signal)
                        continue;

                    auto matrixElement = new DcMatrixElementAlg(device->index(), algPin->index(), signal->index(), "");
                    device->matrix_alg()->add(matrixElement, false);
                }
            };

            makeAlgMatrix(addr, 1);
            makeAlgMatrix(addr + 1, 2);
        }
    }

    void configureMatrixSignals(DcController *device) {
        device->matrix_signals()->removeAll();
        auto crossOutParam = dynamic_cast<DcParamCfg_v2*>(device->getParam(SP_CROSSTABLEDOUT, 0));
        if (!crossOutParam)
            return;

        for(uint16_t i = 0; i < crossOutParam->getSubProfileCount(); ++i) {
            auto doutParam = device->getParam(SP_CROSSTABLEDOUT, i);
            auto dinInternalId = doutParam->value().toUInt();
            if (0xFFFF == dinInternalId)
                continue;

            auto dinSignal = device->getSignal(dinInternalId, DEF_SIG_TYPE_DISCRETE);
            if (!dinSignal)
                continue;

            auto doutInternalId = i;
            auto doutSignal = device->getSignal(doutInternalId, DEF_SIG_TYPE_DISCRETE, DEF_SIG_DIRECTION_OUTPUT);
            if (!doutSignal)
                continue;

            auto element = new DcMatrixElementSignal(device->index(),
                                                     dinSignal->index(), doutSignal->index(), "");
            device->matrix_signals()->add(element, false);
        }
    }
}

DcReadConfigOperation::DcReadConfigOperation(DcController * device, QObject * parent) :
	DcIDeviceOperation("Чтение конфигурации", device, TotalStepsCount, parent),
	m_tempDir(new QTemporaryDir),
	m_filesCount(1),
	m_currentFile(0)
{
	setCheckSoftIdEnabled(true);
	connect(channel().get(), &Channel::progress, this, &DcReadConfigOperation::onChannelProgress);
	gDbManager.beginTransaction(device->index());	
}

DcReadConfigOperation::~DcReadConfigOperation()
{
	gDbManager.endTransaction(device()->index());
	delete m_tempDir;
}

QList<QPair<DcParam*, DcBoard*>> DcReadConfigOperation::failedParams() const
{
	return m_failedParams;
}

QStringList DcReadConfigOperation::failedFiles() const
{
	return m_failedFiles;
}

QHash<int, ParamPackPtr> DcReadConfigOperation::specialParams() const
{
	return m_specialParams;
}

bool DcReadConfigOperation::before()
{
	if (!m_tempDir->isValid()) {
		addError(QString("Не удалось создать временную папку для работы с файлами: %1").arg(m_tempDir->errorString()));
		return false;
	}

	if (!clearDeviceFolder())	
		return false;	

	return true;
}

bool DcReadConfigOperation::exec()
{
	if (!readParams())
		return false;

	if (!readSpecialParams())
		return false;


	{
		QElapsedTimer t;
		t.start();

		FileSystemLoginScope fsls(channel());
		if (!fsls.isValid) {
			addError(QString("Не удалось авторизоваться в файловую систему на устройстве: %1").arg(channel()->errorMsg()));
			return false;
		}

		if (!readFiles())
			return false;

		if (!readConfigArchive())
			return false;

		addInfo(QString("Файлы прочитаны за %1 секунд").arg(t.elapsed() / 1000.0));
	}

	return true;
}

bool DcReadConfigOperation::after()
{
	if (!unpackConfigArchive())
		return false;

	if (!configureSignalBindings())
		return false;

	return true;
}

void DcReadConfigOperation::onChannelProgress(int state)
{
	double totalProgressPerFile = 100.0 / m_filesCount;
	emitProgress(totalProgressPerFile * m_currentFile + totalProgressPerFile * state / 100.0);
}

bool DcReadConfigOperation::clearDeviceFolder()
{
	addInfo("Очистка папки устройства...");

	QFileInfo deviceDbFileInfo(device()->path());
	QDir deviceFolder(deviceDbFileInfo.absolutePath());
	for (auto &fi : deviceFolder.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
		auto path = fi.absoluteFilePath();
		if (fi.isFile() && path == deviceDbFileInfo.absoluteFilePath())
			continue;
		
		if (fi.isDir())
			if (!QDir(path).removeRecursively()) {
				addError(QString("Не удалось удалить полностью папку %1 и содержимое").arg(path));
				return false;
			}

		if (fi.isFile()) {
			QFile file(path);
			if (!file.remove()) {
				addError(QString("Не удалось удалить файл %1: %2").arg(path).arg(file.errorString()));
				return false;
			}
		}
	}

	return true;
}

bool DcReadConfigOperation::readParams()
{
	int totalParamsCount = device()->params_cfg()->size();
	for (size_t i = 0; i < device()->boards()->size(); i++)
		totalParamsCount += device()->boards()->get(i)->params()->size();

	if (!totalParamsCount)
		return true;

	addInfo("Чтение конфигурационых параметров...");
	setCurrentStep(ReadParamStep, totalParamsCount);
	int currentProgress = 0;

	// чтение параметров плат
	for (size_t i = 0; i < device()->boards()->size(); i++) {
		auto board = device()->boards()->get(i);
		for (size_t j = 0; j < board->params()->size(); j++) {
			emitProgress(++currentProgress);
			if (!readParam(board->params()->get(j), board))
				return false;			
		}
	}

	// чтение основных параметров
	for (size_t i = 0; i < device()->params_cfg()->size(); i++) {
		emitProgress(++currentProgress);		
		if (!readParam(device()->params_cfg()->get(i)))
			return false;
	}

	return true;
}

bool DcReadConfigOperation::readSpecialParams()
{
	for (auto param : DcController::specialParams(DcController::BaseParam)) {
		auto dim = channel()->dimension(param);
		if (!dim) {
			addError(QString("Не удалось получить размерность параметра %1: %2").arg(toHex(param)).arg(channel()->errorMsg()));
			return false;
		}

		auto dimSize = dim.subProfileCount();
		auto values = channel()->param(param, 0, dimSize);
		if (Channel::NoError != channel()->errorType()) {
			addError(QString("Не удалось получить занчения параметра %1: %2").arg(toHex(param)).arg(channel()->errorMsg()));
			return false;
		}

		m_specialParams[param] = values;
	}

	return true;
}

bool DcReadConfigOperation::readFiles()
{
	std::map<QString, QString> configFiles;
	for (auto& fm : fileManagersList()) {
		fm->setDevice(device());
		for (auto&& fi : fm->fileInfoList())
			configFiles[fi.devicePath] = fi.localPath;
	}

	if (configFiles.empty())
		return true;

	addInfo("Чтение конфигурационых файлов...");
	setCurrentStep(ReadFilesStep);
	m_filesCount = configFiles.size();
	m_currentFile = 0;	
		
	for (auto &it: configFiles) {
		auto source = it.first;
		auto dest = it.second;
		auto destFolder = QFileInfo(dest).absolutePath();
		if (!QDir(destFolder).mkpath(".")) {
			addError(QString("Не удалось создать папку %1").arg(destFolder));
			return false;
		}

		if (!channel()->downloadFile(source, dest)) {
			addReport(QString("Не удалось загурзить файл %1 с устройства: %2").arg(source).arg(channel()->errorMsg()));
			m_failedFiles << source;
			//addError(QString("Не удалось загурзить файл %1 с устройства: %2").arg(source).arg(channel()->errorMsg()));
			//return false;
		}
		else
			m_downloadedFiles << dest;

		m_currentFile++;
	}	

	return true;
}

bool DcReadConfigOperation::readConfigArchive()
{
	addInfo("Чтение конфигурационного архива...");
	setCurrentStep(ReadArchiveStep);
	m_filesCount = 1;
	m_currentFile = 0;
	m_configArchiveFile.clear();

	QString source = configArchiveDevicePath();
	QString dest = m_tempDir->filePath(QFileInfo(configArchiveDevicePath()).fileName());
	if (!channel()->downloadFile(source, dest) && Channel::FileNotFound != channel()->errorCode()) {
		addError(QString("Не удалось загрузить конфигурационный архив с устройства: %1").arg(channel()->errorMsg()));
		return false;
	}

	if (Channel::NoError == channel()->errorCode())
		m_configArchiveFile = dest;

	if (m_configArchiveFile.isEmpty())
		addInfo("На устройстве нет конфигурационного архива");

	return true;
}

bool DcReadConfigOperation::unpackConfigArchive()
{
	if (m_configArchiveFile.isEmpty())
		return true;

	addInfo("Распаковка конфигурационного архива...");

	// Распаковка и копирование нескачаных файлов в проект.
	QZipReader unzip(m_configArchiveFile, QIODevice::ReadOnly);
	if (QZipReader::NoError != unzip.status()) {
		addError(QString("Не удалось распаковать конфигурационный архив: %1").arg(unzip.status()));
		return false;
	}

	m_archiveDbFile.clear();
	auto fileInfoList = unzip.fileInfoList();
	setCurrentStep(UnpackArchiveStep, fileInfoList.size());
	int currentFI = 0;
	QFileInfo deviceFI(device()->path());
	for (auto &fi : fileInfoList) {
		emitProgress(currentFI++);

		if (fi.isDir) {
			QDir dir(QString("%1/%2").arg(deviceFI.absolutePath()).arg(fi.filePath));
			if (dir.exists())
				continue;

			if (!dir.mkpath(".")) {
				addError(QString("Не удалось создать папку %1").arg(dir.absolutePath()));
				return false;
			}
			if (!QFile::setPermissions(dir.absolutePath(), fi.permissions)) {
				addError(QString("Не удалось установить права на папку %1").arg(dir.absolutePath()));
				return false;
			}
		}

		if (fi.isFile) {
			// Определяем предполагаемый путь в папке устройства
			auto fileName = QString("%1/%2").arg(deviceFI.absolutePath()).arg(fi.filePath);

			// Если это основной файл с параметрами, то распаковываем его в временую папку
			if (fileName == deviceFI.absoluteFilePath()) {
				m_archiveDbFile = m_tempDir->filePath(fi.filePath);
				if (!createFile(m_archiveDbFile, unzip.fileData(fi.filePath), fi.permissions))
					return false;
				continue;
			}

			// Если файл уже подгружали с устройства в папку устройства, ничего не делаем
			bool alreadyDownloaded = false;
			for (auto &file : m_downloadedFiles)
				if (fileName == QFileInfo(file).absoluteFilePath()) {
					alreadyDownloaded = true;
					break;
				}

			if (alreadyDownloaded)
				continue;

			// Все остальные файлы распаковываем в папку устройства.
			if (!createFile(fileName, unzip.fileData(fi.filePath), fi.permissions))
				return false;
		}
	}

	return true;
}

bool DcReadConfigOperation::configureSignalBindings()
{
	addInfo("Настройка привязок сигналов...");

    configureMatrixAlgs(device());
    configureMatrixSignals(device());

	DcController *tempDevice = nullptr;
    if (!m_archiveDbFile.isEmpty()) {
        const int32_t temp_id = 9999;
        tempDevice = gDbManager.load(temp_id, m_archiveDbFile, "");
        if (!tempDevice) {
            addError(QString("Не удалось прочитать конфигурационную базу параметров из конфигурационного архива"));
			return false;
		}
        gDbManager.close(temp_id);
	}
    auto archDevice = std::unique_ptr<DcController>(tempDevice);

    if (!copySignalsNames(device(), archDevice.get())) {
        addError(QString("Не удалось скопировать имена сигналов"));
		return false;
	}

    if (!copyCfcAlgs(device(), archDevice.get())) {
		addError(QString("Не удалось скопировать значения алгоритмов гибкой логики"));
		return false;
	}

    if (!copyMatrixAlgsCfc(device(), archDevice.get())) {
		addError(QString("Не удалось скопировать матрицы алгоритмов гибкой логики"));
		return false;
	}

	return true;
}

bool DcReadConfigOperation::readParam(DcParam * param, DcBoard* board)
{
	// если не установлен атрибут на чтение, пропускаем
	if (!isAttributeInInt(param->flags(), ATTRIBUTE_READ))
		return true;

	// если param специальный параметр, пропускаем
	if (DcController::specialParams().contains(param->addr()))
		return true;

	auto p = channel()->param(param->addr(), param->param(), 1, board ? board->slot() : -1);
	if (p) {
		param->updateValue(p->value<QString>());
		param->reset_param_changes();
		return true;
	}

	// В случаях ошибки от 'устройства' добавляем отчет. Но возвращаем true, чтобы не прерывать всю операцию 
	if (Channel::DeviceErrorType == channel()->errorType()) {
		m_failedParams << qMakePair(param, board);

		QString msg = QString("Параметр %1[%2]").arg(toHex((uint16_t) param->addr())).arg(param->param());
		if (board)
			msg.append(QString(" на плате %1(%2 %3)").arg(board->slot()).arg(board->type()).arg(board->inst()));
		msg.append(QString(" не прочитан: %1").arg(channel()->errorMsg()));
		addReport(msg);
		return true;
	}

	addError(channel()->errorMsg());
	return false;
}

bool DcReadConfigOperation::createFile(const QString & fileName, const QByteArray & data, QFlags<QFileDevice::Permission> permissions)
{
	QFile f(fileName);
	if (!f.open(QIODevice::WriteOnly)) {
		addError(QString("Не удалось создать файл %1: %2").arg(fileName).arg(f.errorString()));
		return false;
	}

	if (f.write(data) < 0) {
		addError(QString("Не удалось записать файл %1: %2").arg(fileName).arg(f.errorString()));
		return false;
	}

	if (!f.setPermissions(permissions)) {
		addError(QString("Не удалос устанвоить права доступа на файл %1: %2").arg(fileName).arg(f.errorString()));
		return false;
	}

	return true;
}
