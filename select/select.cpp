#include "select.h"

#include <string>
#include <vector>

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

namespace widgets {
	Select::Select(QWidget *parent)
		: QMainWindow(parent) {
			ui.setupUi(this);
			ui.step_two_groupbox->setEnabled(false);
		ui.mean_filter_checkbox->setChecked(true);

		ui.mean_shreshold_value_doublespinbox->setValue(0.9);
		ui.average_area_height_spinbox->setRange(1, 10000);
		ui.average_area_width_spinbox->setRange(1,10000);
		ui.average_area_x_offset_spinbox->setRange(0,10000);
		ui.average_area_y_offset_spinbox->setRange(0, 10000);
		ui.average_area_height_spinbox->setValue(700);
		ui.average_area_width_spinbox->setValue(700);
		ui.average_area_x_offset_spinbox->setValue(200);
		ui.average_area_y_offset_spinbox->setValue(200);

		ui.process_bar->hide();

		connect(ui.select_folders_pushbutton, SIGNAL(clicked()), this, SLOT(SelectFoldersButtonClicked()));
		connect(ui.clear_selected_folders_pushbutton, SIGNAL(clicked()), this, SLOT(ClearSelectedFoldersButtonClicked()));
		connect(ui.next_step_pushbutton, SIGNAL(clicked()), this, SLOT(NextStepButtonClicked()));
		connect(&task_thread_, SIGNAL(UpdateStatus(QString)), this, SLOT(UpdateStatusSlot(QString)));
		connect(&task_thread_, SIGNAL(UpdateProgressBar(int, int)), this, SLOT(UpdateProgressBarSlot(int, int)));
	}

	Select::~Select() {

	}

	void Select::SelectFoldersButtonClicked() {
		 QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                             "C:\\",
                                             QFileDialog::ShowDirsOnly
                                             | QFileDialog::DontResolveSymlinks);
		 QString tmp = ui.selected_folders_label->text();
		 tmp.append(dir+"<br>");
		 ui.selected_folders_label->setText(tmp);
	}

	void Select::ClearSelectedFoldersButtonClicked() {
		ui.selected_folders_label->clear();
	}

	void Select::NextStepButtonClicked() {
		if (task_thread_.isRunning()) {
			QMessageBox msgbox;
			msgbox.setText("Please waiting current task done.");
			msgbox.exec();
			return;
		}
		
		data::AttributeTablePtr task_config = data::AttributeTable::create();

		if(ui.step_two_groupbox->isEnabled()) {
			
		} else {
			//ui.step_two_groupbox->setEnabled(true);
			QString folders_string = ui.selected_folders_label->text();
			if (folders_string.isEmpty() || folders_string.isNull()) {
				return;
			}
			QStringList folder_list = folders_string.split("<br>", QString::SkipEmptyParts);
			if (folder_list.size() < 1) {
				return;
			}
			std::vector<std::string> folders;
			for (int i = 0; i < folder_list.size(); i++ ) {
				folders.push_back(folder_list.at(i).toStdString());
			}
			ui.step_one_groupbox->setEnabled(false);
			task_config->insert("TASKNAME", "READFOLDER");
			task_config->insert("FOLDERS", folders);
			task_thread_.set_task(task_config);
		}
	}

	void Select::UpdateStatusSlot(QString msg) {
		ui.status_label->setText(msg);
	}

	void Select::UpdateProgressBarSlot(int value, int max) {
		if (ui.process_bar->isHidden()) {
			ui.process_bar->show();
		}
		if (ui.process_bar->maximum() != max) {
			ui.process_bar->setMaximum(max);
		}
		ui.process_bar->setValue(value);
		if (value == max) {
			ui.process_bar->hide();
		}
	}
}