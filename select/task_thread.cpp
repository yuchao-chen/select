#include "task_thread.h"

#include <string>
#include <vector>
#include <limits>

#include <QDebug>
#include <QFileInfoList>
#include <QFileInfo>

#include <fitsio.h>

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
			WalkThroughDirs(folders);
			FindMsOfFile();
			//for (int i = 0; i < files_.size(); i++) {
			//	qDebug() << files_[i].max << "..." << files_[i].min << "..." << files_[i].mean;
			//}
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

	void TaskThread::WalkThroughDirs(std::vector<std::string> dirs) {
		std::vector<std::string>::const_iterator i = dirs.begin();
		for (; i != dirs.end(); i++) {
			ListFiles(QDir(QString::fromStdString(*i)));
		}
	}
	void TaskThread::FindMsOfFile() {
		std::vector<FileInfo>::iterator i;
		for (i = files_.begin(); i != files_.end(); i++) {
			data::AttributeTablePtr header = data::AttributeTable::create();
			unsigned short *buf = ReadUShortFITS((*i).path, header);
			if (buf) {
				double max = std::numeric_limits<double>::min();
				double min = std::numeric_limits<double>::max();
				double mean = 0.0;
				long npixels = header->get_long("NPIXELS");
				for (int j = 0; j < npixels; j++) {
					if (max < buf[j]) {
						max = buf[j];
					} else if (min > buf[j]) {
						min = buf[j];
					}
					mean += buf[j];
				}
				(*i).max = max;
				(*i).min = min;
				(*i).mean = mean / static_cast<double>(npixels);
				qDebug() << max << "..." << min << "..." << mean;
				delete[] buf;
			}
		}
	}
	unsigned short *TaskThread::ReadUShortFITS(std::string file_path, data::AttributeTablePtr header) {

		fitsfile *fptr = NULL;
		int status = 0, nfound, anynull;
		long naxes[2], fpixel, npixels;
		unsigned short nullval;

		if (fits_open_file(&fptr, file_path.c_str(), READONLY, &status)) {
			qDebug() << "[-] Cannot load test fits" << status;
		}
		if (fits_read_keys_lng(fptr, "NAXIS", 1, 2, naxes, &nfound, &status)) {
			qDebug() << "[-] Cannot read keys" << status;
		}
		
		npixels = naxes[0] * naxes[1];
		fpixel = 1;
		nullval = 0;
		unsigned short *buf = new unsigned short[ npixels ]();
		fits_read_img(fptr, TUSHORT, fpixel, npixels, &nullval, buf, &anynull, &status);
		fits_close_file(fptr, &status);

		header->insert("NPIXELS", npixels);
		return buf;
	}
}