#include "select.h"

#include <string>
#include <vector>

#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

namespace widgets {
	Select::Select(QWidget *parent)
		: QMainWindow(parent) {
		current_step_ = 0;
		config_ = data::AttributeTable::create();

		ui.setupUi(this);
		ui.step_two_groupbox->setEnabled(false);
		ui.step_three_groupbox->setEnabled(false);
		ui.average_area_groupbox->setEnabled(false);
		ui.mean_filter_checkbox->setChecked(true);

		ui.mean_shreshold_value_doublespinbox->setValue(0.9);
		ui.max_shreshold_value_doublespinbox->setValue(0.9);
		ui.min_shreshold_value_doublespinbox->setValue(0.9);
		ui.mean_shreshold_value_doublespinbox->setSingleStep(0.01);
		ui.max_shreshold_value_doublespinbox->setSingleStep(0.01);
		ui.min_shreshold_value_doublespinbox->setSingleStep(0.01);

		ui.average_area_height_spinbox->setRange(1, 10000);
		ui.average_area_width_spinbox->setRange(1,10000);
		ui.average_area_x_offset_spinbox->setRange(0,10000);
		ui.average_area_y_offset_spinbox->setRange(0, 10000);
		ui.average_area_height_spinbox->setValue(700);
		ui.average_area_width_spinbox->setValue(700);
		ui.average_area_x_offset_spinbox->setValue(200);
		ui.average_area_y_offset_spinbox->setValue(200);
		
		plot_ = new QCustomPlot();
		
		plot_->legend->setVisible(true);
		plot_->legend->setFont(QFont("Helvetica",9));

		//plot_->hide();
		plot_->addGraph();
		plot_->graph(0)->setName("Min");
		plot_->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
		//plot_->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue
		plot_->addGraph();
		plot_->graph(1)->setName("Max");
		plot_->graph(1)->setPen(QPen(Qt::red)); // line color red for second graph

		plot_->addGraph();
		plot_->graph(2)->setName("Mean");
		plot_->graph(2)->setPen(QPen(Qt::green));

		plot_->addGraph();
		plot_->addGraph();
		plot_->addGraph();
		plot_->graph(3)->setName("Max Threshold");
		plot_->graph(3)->setPen(QPen(Qt::darkRed));
		plot_->graph(4)->setName("Min Threshold");
		plot_->graph(4)->setPen(QPen(Qt::darkBlue));
		plot_->graph(5)->setName("Mean Threshold");
		plot_->graph(5)->setPen(QPen(Qt::darkGreen));

		plot_->xAxis2->setVisible(true);
		plot_->xAxis2->setTickLabels(false);
		plot_->yAxis2->setVisible(true);
		plot_->yAxis2->setTickLabels(false);

		connect(plot_->xAxis, SIGNAL(rangeChanged(QCPRange)), plot_->xAxis2, SLOT(setRange(QCPRange)));
		connect(plot_->yAxis, SIGNAL(rangeChanged(QCPRange)), plot_->yAxis2, SLOT(setRange(QCPRange)));
		
		plot_->graph(0)->rescaleAxes(true);
		plot_->graph(1)->rescaleAxes(true);
		plot_->graph(2)->rescaleAxes(true);

		plot_->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);

		ui.process_bar->hide();

		connect(ui.select_folders_pushbutton, SIGNAL(clicked()), this, SLOT(SelectFoldersButtonClicked()));
		connect(ui.clear_selected_folders_pushbutton, SIGNAL(clicked()), this, SLOT(ClearSelectedFoldersButtonClicked()));
		connect(ui.next_step_pushbutton, SIGNAL(clicked()), this, SLOT(NextStepButtonClicked()));
		connect(&task_thread_, SIGNAL(UpdateStatus(QString)), this, SLOT(UpdateStatusSlot(QString)));
		connect(&task_thread_, SIGNAL(UpdateProgressBar(int, int)), this, SLOT(UpdateProgressBarSlot(int, int)));
		
		connect(ui.max_filter_checkbox, SIGNAL(stateChanged(int)), this, SLOT(FilterOptionsChangedSlot()));
		connect(ui.min_filter_checkbox, SIGNAL(stateChanged(int)), this, SLOT(FilterOptionsChangedSlot()));
		connect(ui.mean_filter_checkbox, SIGNAL(stateChanged(int)), this, SLOT(FilterOptionsChangedSlot()));

		connect(ui.max_shreshold_value_doublespinbox, SIGNAL(valueChanged(double)), this, SLOT(FilterOptionsChangedSlot()));
		connect(ui.min_shreshold_value_doublespinbox, SIGNAL(valueChanged(double)), this, SLOT(FilterOptionsChangedSlot()));
		connect(ui.mean_shreshold_value_doublespinbox, SIGNAL(valueChanged(double)), this, SLOT(FilterOptionsChangedSlot()));
	
		connect(ui.combine_pushbutton, SIGNAL(clicked()), this, SLOT(CombineSlot()));

		connect(ui.select_output_folder_pushbutton, SIGNAL(clicked()), this, SLOT(SelectOutputFolderSlot()));
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
		if (current_step_ == 0) {
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
		} else if (current_step_ == 1) {
			QString folder = ui.output_folder_lable->text();
			if (folder.isEmpty() || folder.isNull()) {
				return;
			}
			std::vector<std::string> valid_files = config_->get_string_array("VALIDFILES");
			if (valid_files.size() < 1) {
				return;
			}
			task_config->insert("TASKNAME", "TRANSFERFILES");
			task_config->insert("VALIDFILES", valid_files);
			task_config->insert("DESTFOLDER", folder.toStdString());
			task_config->insert("PREFIX", ui.prefix_lineedit->text().toStdString());
			if (ui.rename_files_checkbox->isChecked()) {
				task_config->insert("RENAMEFILES", "ENABLED");
			} else {
				task_config->insert("RENAMEFILES", "DISABLED");
			}
			task_thread_.set_task(task_config);
		}
	}

	void Select::UpdateStatusSlot(QString msg) {
		if (msg == "TASKDONE") {
			if (current_step_ == 0) {
				ui.step_two_groupbox->setEnabled(true);
				ui.step_three_groupbox->setEnabled(true);

				PlotData();
				SelectValidFiles();
				current_step_ = 1;
			}
		} else {
			ui.status_label->setText(msg);
		}
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

	void Select::PlotData() {
		if (plot_->isHidden()) {
			plot_->show();
		}
		std::vector<utils::FileInfo> data = task_thread_.data();
		QVector<double> x(data.size()), min0(data.size()), mean0(data.size()), max0(data.size());
		
		double max1 = std::numeric_limits<double>::min();
		double min1 = std::numeric_limits<double>::max();		
		for (int i = 0; i < data.size(); i++) {
			x[i] = i+1;
			mean0[i] = data[i].mean;
			min0[i] = data[i].min;
			max0[i] = data[i].max;
			if (max1 < max0[i]) {
				max1 = max0[i];
			}
			if (min1 > min0[i]) {
				min1 = min0[i];
			}
		}
		plot_->xAxis->setRange(0, data.size()+1);
		plot_->yAxis->setRange(min1, max1);
		plot_->graph(0)->setData(x, min0);
		plot_->graph(1)->setData(x, max0);
		plot_->graph(2)->setData(x, mean0);

		if (plot_->size().width() < 100) {
			plot_->resize(500, 400);
		}
		qSort(min0.begin(), min0.end());
		qSort(max0.begin(), max0.end());
		qSort(mean0.begin(), mean0.end());

		config_->insert("MAX", max0[max0.size()/2]);
		config_->insert("MIN", min0[min0.size()/2]);
		config_->insert("MEAN", mean0[mean0.size()/2]);

		plot_->replot();
	}

	void Select::SelectValidFiles() {
		std::vector<std::string> valid_files;
		std::vector<utils::FileInfo> data = task_thread_.data();
		double max0 = config_->get_double("MAX");
		double min0 = config_->get_double("MIN");
		double mean0 = config_->get_double("MEAN");

		if (ui.max_filter_checkbox->isChecked()) {
			max0 = max0 * ui.max_shreshold_value_doublespinbox->value();
			QVector<double> y(data.size()), x(data.size());
			for (int i = 0; i < data.size(); i++) {
				x[i] = i+1;
				y[i] = max0;
			}
			plot_->graph(3)->setData(x, y);
		} else {
			max0 = 0.0;
		}

		if (ui.min_filter_checkbox->isChecked()) {
			min0 = min0 * ui.min_shreshold_value_doublespinbox->value();
			QVector<double> y(data.size()), x(data.size());
			for (int i = 0; i < data.size(); i++) {
				x[i] = i+1;
				y[i] = min0;
			}
			plot_->graph(4)->setData(x, y);
		} else {
			min0 = 0.0;
		}

		if (ui.mean_filter_checkbox->isChecked()) {
			mean0 = mean0 * ui.mean_shreshold_value_doublespinbox->value();
			QVector<double> y(data.size()), x(data.size());
			for (int i = 0; i < data.size(); i++) {
				x[i] = i+1;
				y[i] = mean0;
			}
			plot_->graph(5)->setData(x, y);
			qDebug() << mean0;
		} else {
			mean0 = 0.0;
		}
		//qDebug() << mean0 << "................" << max0;
		for (int i = 0; i < data.size(); i++) {
			if ((data[i].max > max0) &&
				(data[i].min > min0) &&
				(data[i].mean > mean0)) {
					valid_files.push_back(data[i].path);
			}
		}
		ui.status_label->setText(QString::number(valid_files.size()) + " files are valid.");
		plot_->replot();
		config_->insert("VALIDFILES", valid_files);

		ExtractCommonFilePrefix();
	}

	void Select::FilterOptionsChangedSlot() {
		SelectValidFiles();
	}

	void Select::CombineSlot() {
		std::vector<std::string> valid_files = config_->get_string_array("VALIDFILES");
		data::AttributeTablePtr config = data::AttributeTable::create();
		config->insert("TASKNAME", "COMBINE");
		config->insert("VALIDFILES", valid_files);
		task_thread_.set_task(config);
	}

	void Select::SelectOutputFolderSlot() {
		QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
			"C:\\",
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);
		ui.output_folder_lable->setText(dir);
	}

	void Select::ExtractCommonFilePrefix() {
		std::vector<std::string> valid_files = config_->get_string_array("VALIDFILES");
		if (valid_files.size() < 2) {
			return;
		}
		QString s1 = QString::fromStdString(valid_files[0]);
		QString s2 = QString::fromStdString(valid_files[valid_files.size()-1]);
		QString s10 = s1.remove(0, s1.lastIndexOf("/")+1);
		QString s20 = s2.remove(0, s2.lastIndexOf("/")+1);
		int pos = 0;
		while (pos > -1) {
			pos = s10.lastIndexOf(QRegExp("\\d+"));
			s10.remove(pos, s10.size());
		}
		ui.prefix_lineedit->setText(s10);
	}

	void Select::closeEvent(QCloseEvent *e) {
		if (!plot_->isHidden()) {
			plot_->close();
		}
		e->accept();
	}
}