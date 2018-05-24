#include "mainwindow.hpp"

#include "ui_untitled.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <thread>

void Genetic::run(equation eq, size_t population_size,
                  equation_processor::population::value_type min,
                  equation_processor::population::value_type max) {
	stop();
	task = std::async(std::launch::async, [this, eq = std::move(eq),
	                                       stop = to_stop.get_future(),
	                                       population_size, min, max] {
		equation_processor proc{eq, population_size, min, max};
		auto population = proc.identity();
		using std::chrono::operator""ms;
		for (size_t id = 0; stop.wait_for(0ms) == std::future_status::timeout; id++) {
			auto stats = proc.step(population);
			emit step(id++, std::move(stats));
			if (stats.min == 0)
				break;
			// std::this_thread::sleep_for(10ms);
		}
	});
}
void Genetic::stop() {
	to_stop = {};
	task = {};
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent}, ui{std::make_unique<Ui::MainWindow>()},
      chartMin{this}, chartMean{this} {
	ui->setupUi(this);
	seriesMin.setName("min");
	chartMin.chart()->addSeries(&seriesMin);
	chartMin.chart()->setAxisX(&xAxisMin, &seriesMin);
	chartMin.chart()->setAxisY(&yAxisMin, &seriesMin);
	seriesMean.setName("mean");
	chartMean.chart()->addSeries(&seriesMean);
	chartMean.chart()->setAxisX(&xAxisMean, &seriesMean);
	chartMean.chart()->setAxisY(&yAxisMean, &seriesMean);
	ui->verticalLayout->addWidget(&chartMin);
	ui->verticalLayout->addWidget(&chartMean);
	qRegisterMetaType<equation_processor::stat>("equation_processor::stat");
	qRegisterMetaType<size_t>("size_t");
	connect(ui->solveButton, &QPushButton::clicked, this,
	        [this, clicked = false]() mutable {
		        if (!clicked) {
			        equation eq;
			        std::istringstream is{ui->inputEdit->text().toStdString() + "\n"};
			        if (is >> eq) {
				        genetic = std::make_unique<Genetic>();
				        connect(
				            genetic.get(), &Genetic::step, this,
				            [this, eq](size_t id, equation_processor::stat value) {
					            static long long maxMin;
					            static long long maxMean;
					            static long long minMin;
					            static equation_processor::population::substit_type minValues;
					            if (id == 0) {
						            seriesMin.clear();
						            seriesMean.clear();
						            maxMin = 0;
						            maxMean = 0;
						            minMin = std::numeric_limits<long long>::max();
						            minValues = {};
					            }
					            maxMin = std::max(maxMin, value.min);
					            if (value.min < minMin) {
						            minMin = value.min;
						            minValues = value.values;
					            }
					            maxMean = std::max(maxMean, value.mean);
					            seriesMean.append(id, value.mean);
					            seriesMin.append(id, value.min);
					            // std::cout << value.min << ' ' << value.mean << std::endl;
					            if (seriesMin.count() > 100) {
						            seriesMin.remove(0);
						            seriesMean.remove(0);
					            }
					            xAxisMin.setRange(seriesMin.points()[0].x(),
					                              seriesMin.points()[0].x() +
					                                  seriesMin.count());
					            yAxisMin.setRange(0, maxMin);
					            xAxisMean.setRange(seriesMean.points()[0].x(),
					                               seriesMean.points()[0].x() +
					                                   seriesMean.count());
					            yAxisMean.setRange(0, maxMean);
					            {
						            QString lbl = QString::number(minMin) + "[";
						            for (auto &a : minValues)
							            lbl += QString::number(a) + ", ";
						            lbl += ']';
						            ui->bestLabel->setText(lbl);
					            }
					            if (value.min == 0) {
						            for (auto &a : value.values)
							            std::cout << a << ' ';
						            std::cout << std::endl;
					            }
				            },
				            Qt::QueuedConnection);
				        genetic->run(std::move(eq), ui->sizeBox->value(),
				                     ui->minBox->value(), ui->maxBox->value());
			        }
		        } else {
			        genetic = {};
		        }
		        clicked = !clicked;
	        });
}
MainWindow::~MainWindow() {}
