#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtWidgets/QMainWindow>
#include <future>
#include <memory>

#include "solver/genetic.hpp"

namespace Ui {
class MainWindow;
}

class Genetic : public QObject {
	Q_OBJECT
	std::future<void> task;
	std::promise<void> to_stop;

public:
	void run(equation eq, size_t population_size,
	         equation_processor::population::value_type min,
	         equation_processor::population::value_type max);
	void stop();
signals:
	void step(size_t id, equation_processor::stat);
};

class MainWindow : public QMainWindow {
	Q_OBJECT
	std::unique_ptr<Ui::MainWindow> ui;
	std::unique_ptr<Genetic> genetic;
	QtCharts::QLineSeries seriesMin;
	QtCharts::QValueAxis xAxisMin;
	QtCharts::QValueAxis yAxisMin;
	QtCharts::QLineSeries seriesMean;
	QtCharts::QValueAxis xAxisMean;
	QtCharts::QValueAxis yAxisMean;
	QtCharts::QChartView chartMin;
	QtCharts::QChartView chartMean;

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();
};

#endif /* end of include guard: MAINWINDOW_HPP */
