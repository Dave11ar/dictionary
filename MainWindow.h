#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DictionaryWorker.h>
#include <QMainWindow>
#include <QObject>
#include <QLineEdit>
#include <QTextBrowser>
#include <QThread>
#include <memory>
#include <unordered_set>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void paintEvent(QPaintEvent*) override;
private slots:
    void inputChanged();
    void outputChanged();

private:
    std::unique_ptr<Ui::MainWindow> ui;
    DictionaryWorker worker;
    std::unordered_set<std::string> words;
    uint64_t version;
};
#endif // MAINWINDOW_H
