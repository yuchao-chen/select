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

	private:
		void WalkDirs(std::vector<std::string> dirs);
		void ListFiles(QDir dir);

		data::AttributeTablePtr task_config_;
		std::vector<FileInfo> files_;
	};
}
#endif