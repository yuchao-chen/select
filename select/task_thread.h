#ifndef TASK_THREAD_H
#define TASK_THREAD_H

#include <QThread>
#include <QDir>

#include "attribute_table.h"

namespace utils {
	struct FileInfo {
		std::string path;
		double mean;
		double max;
		double min;
	};

	class TaskThread: public QThread {
		Q_OBJECT
	public:
		TaskThread();
		~TaskThread();

		void set_task(data::AttributeTablePtr task_config);
		void run();
		
		std::vector<FileInfo> data() {
			return files_;
		}

	signals:
		void UpdateStatus(QString);
		void UpdateProgressBar(int, int);
	
	private:
		void WalkThroughDirs(std::vector<std::string> dirs);
		void ListFiles(QDir dir);
		void FindMsOfFile();
		void CombineFiles();

		unsigned short *ReadUShortFITS(std::string file_path, data::AttributeTablePtr header);
		//void ReadFITSHeader(std::string file_path, data::AttributeTablePtr header);
		data::AttributeTablePtr ReadFITSHeader(std::string file_path);
		void WriteFloatFITS(float *data, data::AttributeTablePtr header);
		data::AttributeTablePtr task_config_;
		std::vector<FileInfo> files_;
	};
}
#endif