#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(&cui,
            SIGNAL(f3_launcher_status_changed(f3_launcher_status)),
            this,
            SLOT(on_cui_status_changed(f3_launcher_status)));
    checking = false;
    move((QApplication::desktop()->width() - width()) / 2,
         (QApplication::desktop()->width() - width()) / 2);
    setFixedSize(width(), height());
    clearStatus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showStatus(const QString &string)
{
    ui->statusBar->showMessage(string);
}

void MainWindow::clearStatus()
{
    ui->statusBar->showMessage("Ready");
    ui->labelSpace->clear();
    ui->labelSpeed->clear();
    ui->progressBar->setValue(0);
    ui->progressBar->setTextVisible(false);
}

void MainWindow::on_cui_status_changed(f3_launcher_status status)
{
    switch(status)
    {
        case f3_launcher_ready:
            showStatus("Ready.");
            break;
        case f3_launcher_running:
            showStatus("Checking... Please wait...");
            break;
        case f3_launcher_finished:
        {
            f3_launcher_report report = cui.getReport();
            if (report.success)
                showStatus("Finished (without error).");
            else
                showStatus("Finished.");
            ui->labelSpace->setText(QString("Free Space: ")
                                    .append(report.ReportedFree)
                                    .append("\nActual: ")
                                    .append(report.ActualFree)
                                    );
            ui->labelSpeed->setText(QString("Read speed: ")
                                    .append(report.ReadingSpeed)
                                    .append("\nWrite speed: ")
                                    .append(report.WritingSpeed)
                                    );
            ui->progressBar->setValue(report.availability * 100);
            ui->progressBar->setTextVisible(true);
            break;
        }
        case f3_launcher_stopped:
            showStatus("Stopped.");
            break;
        case f3_launcher_path_incorrect:
            QMessageBox::critical(this,"Path error",
                                  "Device path not found.\n"
                                  "Please try mounting it correctly.");
            break;
        case f3_launcher_no_cui:
            QMessageBox::critical(this,"No f3 program",
                                  "Cannot found f3read/f3write.\n"
                                  "Please install f3 first.");
            showStatus("No enough space for test.");
            break;
        case f3_launcher_no_permission:
            QMessageBox::warning(this,"Permission denied",
                              "Cannot write to device.\n"
                              "Try to re-run with sudo.");
            showStatus("No enough space for test.");
            break;
        case f3_launcher_no_space:
            QMessageBox::information(this,"No space",
                              "No enough space for checking.\n"
                              "Please delete some file, or format the device and try again.");
            showStatus("No enough space for test.");
            break;
        default:
            showStatus("An error occurred.");
    }
    if (status == f3_launcher_running)
    {
        checking = true;
        ui->buttonCheck->setText("Stop");
    }
    else
    {
        checking = false;
        ui->buttonCheck->setText("Check!");
    }
}

void MainWindow::on_buttonCheck_clicked()
{
    QString inputPath = ui->textDevPath->text().trimmed();
    if (inputPath.isEmpty())
    {
        QMessageBox::warning(this,"Warning","Please input the device path!");
        return;
    }
    if (checking)
    {
        showStatus("Stopped.");
        cui.stopCheck();
    }
    else
    {
        clearStatus();
        cui.startCheck(inputPath);
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (checking)
    {
        if (QMessageBox::question(this,"Quit F3","The program is still running a check.\n"
            "Quit anyway?",QMessageBox::Yes,QMessageBox::No) != QMessageBox::Yes)
            event->ignore();
        else
            cui.stopCheck();
    }
}

void MainWindow::on_buttonExit_clicked()
{
    if (checking)
    {
        cui.stopCheck();
        checking = false;
    }
    this->close();
}

void MainWindow::on_buttonSelectPath_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this,"Choose Device Path");
    if (!path.isEmpty())
        ui->textDevPath->setText(path);

}
