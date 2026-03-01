#include "DcWriteConfigOperation.h"

#include <qdiriterator.h>
#include <qtemporarydir.h>
#include <qdebug.h>

#include <dpc/sybus/channel/Channel.h>
#include <dpc/sybus/utils.h>
#include <utils/qzipwriter_p.h>

#include <file_managers/IFileManager.h>

using namespace Dpc::Sybus;

namespace {
	enum OperationSteps {
		WriteFilesStep = 0,
		WriteParamsStep,
		SaveRestartStep,

		TotalStepsCount
	};

	const uint8_t RESTART_FLAG_VALUE = 1;

	void fillParam(DcController *device, uint16_t addr, const QString &value)
	{
		auto p = dynamic_cast<DcParamCfg_v2*>(device->getParam(addr, 0));
		if (!p)
			return;

		for (size_t i = 0; i < p->getProfileCount(); i++)
			for (size_t j = 0; j < p->getSubProfileCount(); j++) {
				auto param = device->getParam(addr, i * PROFILE_SIZE + j);
				param->updateValue(value);
			}
	}

	DcAlgInternal *getAlgByIO(DcController *pcontr, int32_t srcid)
	{
		for (uint32_t i = 0; i < pcontr->algs_internal()->size(); i++) {
			auto palg = pcontr->algs_internal()->get(i);
			for (uint32_t j = 0; j < palg->ios()->size(); j++) {
				auto pcfcio = palg->ios()->get(j);
				if (pcfcio->index() == srcid)
					return palg;
			}
		}

		return nullptr;
	}

	bool compileAlgMatrixElement(DcController *controller, int32_t srcid, int32_t dstid) {
		if ((srcid < 1) || (dstid < 1))
			return false;
		DcAlgInternal *palg = getAlgByIO(controller, srcid);
		if (palg == nullptr)
			return false;

		DcParamCfg *pparam = nullptr;
		DcAlgIOInternal *pios = palg->ios()->getById(srcid);
		uint32_t addr = palg->property("addr").toUInt(nullptr, 16);
		if (!addr) {
			int32_t pos = palg->position() * PROFILE_SIZE + pios->pin();
			pparam = controller->params_cfg()->get(SP_CROSSTABLE, pos);
		}
		else
			pparam = controller->params_cfg()->get(addr + pios->direction() - 1, pios->pin());
		if (pparam == nullptr)
			return false;

		DcSignal *psignal = controller->getSignal(dstid);
		if (!psignal)
			return false;

		pparam->updateValue(QString::number(psignal->internalId()));
		return true;
	}

	bool compileSignalMatrixElement(DcController *controller, int32_t outid, int32_t dinid) {
		if ((outid < 1) || (dinid < 1))
			return false;

		DcSignal *psignal = controller->getSignal(outid);
		if (!psignal)
			return false;

		//if (psignal->direction() != DEF_SIG_DIRECTION_OUTPUT) // ИНАЧЕ не комплилируются привязки ко входам алгоритмов
		//	return false;

		DcSignal *pdinsignal = controller->getSignal(dinid);

		DcParamCfg *pparam = controller->params_cfg()->get(SP_CROSSTABLEDOUT, psignal->internalId());
		if (pparam == nullptr)
			return false;
		pparam->updateValue(QString::number(pdinsignal->internalId()));
		return true;
	}
}

DcWriteConfigOperation::DcWriteConfigOperation(DcController * device, QObject * parent) :
	DcIDeviceOperation("Запись конфигурации", device, TotalStepsCount, parent),
	m_tempDir(new QTemporaryDir),
	m_filesCount(1),
	m_currentFile(0)
{
	setCheckSoftIdEnabled(true);
	connect(channel().get(), &Channel::progress, this, &DcWriteConfigOperation::onChannelProgress);
}

DcWriteConfigOperation::~DcWriteConfigOperation()
{
	delete m_tempDir;
}

QString DcWriteConfigOperation::ipAddress() const
{
    return m_ip;
}

bool DcWriteConfigOperation::compileBindindgs(DcController *device)
{
    fillParam(device, SP_CROSSTABLE, QString::number(0xFFFF));
    fillParam(device, SP_CROSSTABLEDOUT, QString::number(0xFFFF));

    for (uint32_t i = 0; i < device->algs_internal()->size(); i++) {
        DcAlgInternal *alg = device->algs_internal()->get(i);
        uint32_t addr = alg->property("addr").toUInt(nullptr, 16);
        if (!addr)
            continue;

        fillParam(device, addr, QString::number(0xFFFF));
        fillParam(device, addr + 1, QString::number(0xFFFF));
    }

    uint32_t algsize = device->matrix_alg()->size();
    for (uint32_t i = 0; i < algsize; i++) {
        DcMatrixElementAlg *palg = device->matrix_alg()->get(i);
        if (!compileAlgMatrixElement(device, palg->src(), palg->dst())) {
            return false;
        }
    }

    uint32_t sigsize = device->matrix_signals()->size();
    for (uint32_t i = 0; i < sigsize; i++) {
        DcMatrixElementSignal *palg = device->matrix_signals()->get(i);
        if (!compileSignalMatrixElement(device, palg->dst(), palg->src())) {
            return false;
        }
    }

    return true;
}

bool DcWriteConfigOperation::before()
{
	if (!complieCrossTables()) {
		addError("Ошибка компиляции привязок");
		return false;
	}

	if (!m_tempDir->isValid()) {
		addError(QString("Не удалось создать временную директорию для работы с файлами: %1").arg(m_tempDir->errorString()));
		return false;
	}

	if (!packConfigArchive())
		return false;

	return true;
}

bool DcWriteConfigOperation::exec()
{
	if (!setPrepareWorkMode())
		return false;

	if (!prepareRestartFlag())
		return false;

	if (!uploadFiles())
		return false;

	if (!writeParams())
		return false;

	if (!readIpAddress())
		return false;

	if (!checkRestartFlag())
		return false;

	if (!saveAndRestart())
		return false;	

	return true;
}

void DcWriteConfigOperation::onChannelProgress(int state)
{
	double totalProgressPerFile = 100.0 / m_filesCount;
	emitProgress(totalProgressPerFile * m_currentFile + totalProgressPerFile * state / 100.0);
}

bool DcWriteConfigOperation::complieCrossTables()
{
	addInfo("Компиляция таблиц привязки...");

    return compileBindindgs(device());
}

bool DcWriteConfigOperation::packConfigArchive()
{
	addInfo("Упаковка конфигурационного архива...");

	QString fileName = m_tempDir->filePath(QFileInfo(configArchiveDevicePath()).fileName());
	QZipWriter zip(fileName);
	if (zip.status() != QZipWriter::NoError) {
		addError(QString("Не удалось создать файл конфигурационного архива %1: %2").arg(fileName).arg(zip.status()));
		return false;
	}

	zip.setCompressionPolicy(QZipWriter::AutoCompress);
	QDir dir(QFileInfo(device()->path()).absolutePath());
	dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
	QDirIterator it(dir, QDirIterator::Subdirectories);
	while (it.hasNext()) {
		auto path = it.next();
		auto fi = it.fileInfo();
		auto relativePath = dir.relativeFilePath(fi.absoluteFilePath());

		if (fi.isDir()) {
			zip.setCreationPermissions(QFile::permissions(path));
			zip.addDirectory(relativePath);
			if (zip.status() != QZipWriter::NoError) {
				addError(QString("Не удалось создать папку %1 в конфигурационном архиве: %2").arg(relativePath).arg(zip.status()));
				return false;
			}
		}
		else if (fi.isFile()) {
			QFile file(path);
			zip.setCreationPermissions(file.permissions());

            // Костыль, временый файл удаляется к началу архивирования
            if(!file.exists())
                continue;

			zip.addFile(relativePath, &file);
			if (zip.status() != QZipWriter::NoError) {
				addError(QString("Не удалось упаковать файл %1 в конфигурационный архив: %2").arg(path).arg(zip.status()));
				return false;
			}
		}
	}

	m_configArchiveFile = fileName;
	return true;
}

bool DcWriteConfigOperation::setPrepareWorkMode()
{
	if (!channel()->setDeviceWorkMode(Channel::Prepare)) {
		QString msg = QString("Не удалось перевести устройство в режим подготовки: %1").arg(channel()->errorMsg());
		if (Channel::BadID != channel()->errorCode()) {
			addError(msg);
			return false;
		}

		addInfo(msg);
	}

	return true;
}

bool DcWriteConfigOperation::prepareRestartFlag()
{
	addInfo("Подготовка флага перезагрузки...");

	auto item = ParamPack::create(T_BYTE, SP_RESET_DEVICE_FLAG);
	item->appendValue(RESTART_FLAG_VALUE);
	if (!channel()->setParam(item)) {
		addError(QString("Не удалось подготовить флаг перезагрузки: %1").arg(channel()->errorMsg()));
		return false;
	}

	return true;
}

bool DcWriteConfigOperation::checkRestartFlag()
{
	addInfo("Проверка флага перезагрузки...");

	auto p = channel()->param(SP_RESET_DEVICE_FLAG);
	if (!p) {
		addError(QString("Не удалось проверить флаг перезагрузки: %1").arg(channel()->errorMsg()));
		return false;
	}

	return p->value<uint8_t>() == RESTART_FLAG_VALUE;
}

bool DcWriteConfigOperation::uploadFiles()
{
	std::map<QString, QString> configFiles;
	for (auto& fm : fileManagersList()) {
		fm->setDevice(device());
		for (auto&& fi : fm->fileInfoList())
			configFiles[fi.localPath] = fi.devicePath;
	}
	configFiles[m_configArchiveFile] = configArchiveDevicePath();

	addInfo("Запись конфигурационых файлов...");
	setCurrentStep(WriteFilesStep);
	m_filesCount = configFiles.size();
	m_currentFile = 0;

	QElapsedTimer t;
	t.start();

	FileSystemLoginScope fsls(channel());
	if (!fsls.isValid) {
		addError(QString("Не удалось авторизоваться в файловую систему на устройстве: %1").arg(channel()->errorMsg()));
		return false;
	}

	for (auto &it : configFiles) {
		auto source = it.first;
		auto dest = it.second;

		auto destDir = dest.left(dest.lastIndexOf('/'));
		if (!destDir.isEmpty() && !channel()->mkdirectory(destDir)) {
			addError(QString("Не удалось создать папку %1 на устройстве: %2").arg(destDir).arg(channel()->errorMsg()));
			return false;
		}

		if (!channel()->uploadFile(source, dest)) {
			addError(QString("Не удалось загурзить файл %1 на устройство: %2").arg(source).arg(channel()->errorMsg()));
			return false;
		}

		m_currentFile++;
	}	

	addInfo(QString("Файлы записаны за %1 секунд").arg(t.elapsed() / 1000.0));

	return true;
}

bool DcWriteConfigOperation::writeParams()
{
	int totalParamsCount = device()->params_cfg()->size();
	for (size_t i = 0; i < device()->boards()->size(); i++)
		totalParamsCount += device()->boards()->get(i)->params()->size();

	if (!totalParamsCount)
		return true;

	addInfo("Запись конфигурационых параметров...");
	setCurrentStep(WriteParamsStep, totalParamsCount);
	int currentProgress = 0;

	// запись основных параметров
	for (size_t i = 0; i < device()->params_cfg()->size(); i++) {
		emitProgress(++currentProgress);
		if (!writeParam(device()->params_cfg()->get(i)))
			return false;
	}

	// запись параметров плат
	for (size_t i = 0; i < device()->boards()->size(); i++) {
		auto board = device()->boards()->get(i);
		if (!board->params()->size())
			continue;

		if (!channel()->login(board->slot())) {
			addError(QString("Не удалось авторизоваться на плате %1(%2 %3): %4").arg(board->slot()).arg(board->type()).arg(board->inst()).arg(channel()->errorMsg()));
			return false;
		}

		for (size_t j = 0; j < board->params()->size(); j++) {
			emitProgress(++currentProgress);
			if (!writeParam(board->params()->get(j), board))
				return false;
		}

		m_writtenBoards << board;
	}	

	return true;
}

bool DcWriteConfigOperation::readIpAddress()
{
	auto p = channel()->param(SP_IP4_ADDR);
	if (!p && channel()->errorCode() != Channel::BadID) {
		addError(QString("Не удалось получить значение Ip-Адреса : %1").arg(channel()->errorMsg()));
		return false;
	}

	if (p)
		m_ip = p->value<QString>();
	
	return true;
}

bool DcWriteConfigOperation::saveAndRestart()
{
	setCurrentStep(SaveRestartStep, m_writtenBoards.size() + 1);
	int currentProgress = 0;
	for (size_t i = 0; i < m_writtenBoards.size(); i++) {
		auto board = m_writtenBoards.at(i);
		if (!saveConfig(board))
			return false;

		if (!restart(board))
			return false;

		emitProgress(++currentProgress);
	}

	if (!saveConfig())
		return false;

	if (!restart())
		return false;

	emitProgress(++currentProgress);
	return true;
}

bool DcWriteConfigOperation::writeParam(DcParam * param, DcBoard * board)
{
	// если не установлен атрибут на запись, пропускаем
	if (!isAttributeInInt(param->flags(), ATTRIBUTE_WRITE))
		return true;

	// если это специальный параметр, пропускаем
	if (DcController::specialParams().contains(param->addr()))
		return true;

	// КОСТЫЛЬ!!! частный случай, чтобы не менять пароли!
	if (param->addr() == SP_PASSWORDMAN || param->addr() == SP_PASSWORDNET)
		return true;

	auto p = ParamPack::create(param->type(), param->addr(), param->param());
	p->appendValue(param->value());
	if (channel()->setParam(p, board ? board->slot() : -1))
		return true;

	// В случаях ошибки от 'устройства' добавляем отчет. Но возвращаем true, чтобы не прерывать всю операцию 
	if (Channel::DeviceErrorType == channel()->errorType()) {
		QString msg = QString("Параметр %1[%2]").arg(toHex((uint16_t)param->addr())).arg(param->param());
		if (board)
			msg.append(QString(" на плате %1(%2 %3)").arg(board->slot()).arg(board->type()).arg(board->inst()));
		msg.append(QString(" не записан: %1").arg(channel()->errorMsg()));
		addReport(msg);
		return true;
	}

	addError(channel()->errorMsg());
	return false;
}
