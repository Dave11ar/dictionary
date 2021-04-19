#include "MainWindow.h"
#include "ui_MainWindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , worker(".." + fileSeparator + "Data" + fileSeparator + "words.txt")
    , words()
    , version(1)
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
    version++;
    words.clear();
    ui->textBrowser->setText("");
    std::string input = ui->lineEdit->text().toStdString();

    if (!input.empty()) {
        worker.setInput(std::optional<std::string>(input), version);
    }
}

void MainWindow::outputChanged()
{
    Result result = worker.getOuput();
    if (version != result.version || words.count(result.word) != 0) {
        return;
    }

    words.insert(result.word);
    ui->textBrowser->append(QString::fromStdString(result.word));
}
