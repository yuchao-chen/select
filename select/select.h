#ifndef SELECT_H
#define SELECT_H

#include <QtWidgets/QMainWindow>

#include "ui_select.h"
#include "task_thread.h"
namespace widgets {
	class Select : public QMainWindow {
		Q_OBJECT

	public:
		Select(QWidget *parent = 0);
		~Select();

	private slots:
		void SelectFoldersButtonClicked();
		void ClearSelectedFoldersButtonClicked();
		void NextStepButtonClicked();
		
	private:
		Ui::selectClass ui;
		utils::TaskThread task_thread_;
	};
}
#endif // SELECT_H
