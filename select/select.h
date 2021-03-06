#ifndef SELECT_H
#define SELECT_H

#include <QtWidgets/QMainWindow>

#include "ui_select.h"
#include "task_thread.h"
#include "qcustomplot.h"

namespace widgets {
	class Select : public QMainWindow {
		Q_OBJECT

	public:
		Select(QWidget *parent = 0);
		~Select();
	
	protected:
		void closeEvent(QCloseEvent *);

	private slots:
		void SelectFoldersButtonClicked();
		void ClearSelectedFoldersButtonClicked();
		void NextStepButtonClicked();
		void UpdateStatusSlot(QString);
		void UpdateProgressBarSlot(int, int);
		void FilterOptionsChangedSlot();
		void CombineSlot();
		void SelectOutputFolderSlot();
	private:
		void PlotData();
		void SelectValidFiles();
		void ExtractCommonFilePrefix();

		Ui::selectClass ui;
		utils::TaskThread task_thread_;
		int current_step_;
		QCustomPlot *plot_;
		data::AttributeTablePtr config_;
	};
}
#endif // SELECT_H
