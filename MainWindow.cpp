#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , worker("words.txt")
{
    ui->setupUi(this);

    connect(&worker, &DictionaryWorker::ready, this, &MainWindow::outputChanged);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &MainWindow::inputChanged);
}

MainWindow::~MainWindow() {}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
}

void MainWindow::inputChanged()
{
    std::string input = ui->lineEdit->text().toStdString();
    ui->textBrowser->setText("");
    version++;
    if (!input.empty()) {
        worker.setInput(std::optional<std::string>(input), version);
    }
}

void MainWindow::outputChanged()
{
    Result result = worker.getOuput();
    if (version != result.version) {
        return;
    }
    ui->textBrowser->append(QString::fromStdString(result.word));
}
