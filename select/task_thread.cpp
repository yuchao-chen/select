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
			emit UpdateStatus(QString::number(files_.size()) + " files found.");
			FindMsOfFile();
			emit UpdateStatus("TASKDONE");
		} else if (task_name == "COMBINE") {
			CombineFiles();
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
		int index = 0;
		int size = files_.size();
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
					mean += buf[j] / static_cast<double>(npixels);
				}
				(*i).max = max;
				(*i).min = min;
				(*i).mean = mean;
				//qDebug() << max << "..." << min << "..." << mean;
				delete[] buf;
				index++;
				emit UpdateProgressBar(index, size);
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
		header->insert("WIDTH", naxes[0]);
		header->insert("HEIGHT", naxes[1]);
		return buf;
	}

	void TaskThread::CombineFiles() {
		std::vector<std::string> files = task_config_->get_string_array("VALIDFILES");
		int size = files.size();
		double d_size = static_cast<double>(size);
		data::AttributeTablePtr header0 = ReadFITSHeader(files[0]);
		long npixels = header0->get_long("NPIXELS");
		float *combined_file = new float[npixels]();

		for (int i = 0; i < size; i++) {
			data::AttributeTablePtr header = data::AttributeTable::create();
			unsigned short *buf = ReadUShortFITS(files[i], header);
			if (npixels == header->get_long("NPIXELS")) {
				for (int j = 0; j < npixels; j++) {
					combined_file[j] += buf[j] / d_size;
				}
			}
			qDebug() << i << "..........." << size;
			emit UpdateProgressBar(i+1, size);
			delete[] buf;
		}
		WriteFloatFITS(combined_file, header0);
		delete[] combined_file;
	}

	data::AttributeTablePtr TaskThread::ReadFITSHeader(std::string file_path) {
		data::AttributeTablePtr header = data::AttributeTable::create();
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
		fits_close_file(fptr, &status);

		header->insert("NPIXELS", npixels);
		header->insert("WIDTH", naxes[0]);
		header->insert("HEIGHT", naxes[1]);
		return header;
	}

	void TaskThread::WriteFloatFITS(float *data, data::AttributeTablePtr header) {
	
		std::string path = "C:/Workspace/flat.fits";
		fitsfile *fptr;

		int status = 0;
		int bitpix = FLOAT_IMG;
		long naxis = 2;
		long naxes[2];

		naxes[0] = header->get_long("WIDTH");
		naxes[1] = header->get_long("HEIGHT");

		remove(path.c_str());

		if (fits_create_file(&fptr, path.c_str(), &status)) {
			qDebug() << "Cannot create file: Error " << status;
		}

		if (fits_create_img(fptr, bitpix, naxis, naxes, &status)) {
			qDebug() << "Cannot create image: Error " << status;
		}

		long fpixel = 1;
		long nelements = naxes[0] * naxes[1];
		if (fits_write_img(fptr, TFLOAT, fpixel, nelements, data, &status)) {
			qDebug() << "Cannot write file: Error " << status;
		}

		if (fits_close_file(fptr, &status)) {
			qDebug() << "Cannot close file: Error " << status;
		}
	}
}