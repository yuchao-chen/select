#include "task_thread.h"

#include <string>
#include <vector>

#include <QDebug>
#include <QFileInfoList>
#include <QFileInfo>

namespace utils {
	TaskThread::TaskThread()
		:task_config_(data::AttributeTable::create()) {
	}

	TaskThread::~TaskThread() {
	}

	void TaskThread::run() {
		if (!task_config_ || task_config_->empty()){
			return;
		}
		std::string task_name = task_config_->get_string("TASKNAME");
		if (task_name == "READFOLDER") {
			std::vector<std::string> folders = task_config_->get_string_array("FOLDERS");
			if (folders.size() < 1) {
				return;
			}
			files_.clear();
			WalkDirs(folders);
		}
	}

	void TaskThread::set_task(data::AttributeTablePtr task_config) {
		task_config_->set(task_config->map_view());
		start();
	}

	void TaskThread::ListFiles(QDir dir) {
		if (!dir.exists()) {
			qDebug() << "Not Exist";
		}
		//QFileInfoList entries = dir.entryInfoList( QDir::NoDotAndDotDot |
		//		QDir::Dirs | QDir::Files );
		QFileInfoList entries = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
		foreach (QFileInfo entry_info, entries) {
			if (!entry_info.isDir()) {
				QString path = entry_info.absoluteFilePath();
				FileInfo file;
				file.path = path.toStdString();
				files_.push_back(file);
			}
		}
	}
	void TaskThread::WalkDirs(std::vector<std::string> dirs) {
		std::vector<std::string>::const_iterator i = dirs.begin();
		for (; i != dirs.end(); i++) {
			ListFiles(QDir(QString::fromStdString(*i)));
		}
	}
}