#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include <QTextBrowser>
#include <QPushButton>
#include <QVariant>
#include <time.h>
#include <QLayout>
#include <QMouseEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow(){
    delete ui;
}

enum class GameResult{
    Won,
    Lost,
};


QMessageBox::StandardButton restart_game(GameResult game_result){
    QMessageBox *message = new QMessageBox;
    message->setText(game_result == GameResult::Won ? "You won. \nRestart the game?" : "You lose. \nRestart the game?");
    message->setStandardButtons(QMessageBox::Yes | QMessageBox::Close);
    message->setDefaultButton(QMessageBox::Yes);
    int answer = message->exec();
    return static_cast<QMessageBox::StandardButton>(answer);
}

class My_PushButton: public QPushButton{
    Q_OBJECT
public:
    Ui::MainWindow* form_pointer;   // This is a pointer to the form to refer to elements

    /* val - variable that will store either the number of mines in the neighborhood, or the mine itself
       ptr_n_b - pointer to the number of buttons. It will be used to complete the game
       ptr_n_m - pointer to the number of mines. Will be used to end the game
       max_flags - pointer to the number of flags left */

    int *ptr_n_b = nullptr, *ptr_n_m = nullptr, *max_flags = nullptr;
    int val;
    bool open = false, flag_raised = false;
    My_PushButton(int val = 0, int max_flags = 0): QPushButton(), val(val){
        setFixedHeight(25);
        setFixedWidth(25);
        setIconSize(QSize(25, 24));
        setStyleSheet("background-color: rgb(146, 36, 255)");
    }
private slots:
    void mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::RightButton) {
            this->flag_raised = !this->flag_raised;
            if (this->flag_raised) {
                if (*this->max_flags <= 0) {
                    this->flag_raised = false;
                } else if (!this->open) {
                    this->setText("");
                    this->setStyleSheet("background-color: rgb(209, 173, 245)");
                    this->setIcon(QIcon(R"(flag.png)"));
                    --*this->max_flags;
                }
            } else if (!this->open) {
                this->setText("");
                this->setIcon(QIcon());
                this->setStyleSheet("background-color: rgb(146, 36, 255)");
                ++*this->max_flags;
            }
        } else if (!this->flag_raised) {
            if (this->val == 9) {
                this->setText("");
                this->setStyleSheet("background-color: rgb(209, 173, 245)");
                this->setIcon(QIcon(R"(bomb.png)"));
                if (auto win = QApplication::activeWindow()) win->setEnabled(false);
                if (restart_game(GameResult::Lost) == QMessageBox::Yes) {
                    if (auto win = QApplication::activeWindow()) win->setEnabled(true);
                    this->form_pointer->pushButton->click();
                    } else {
                        QApplication::quit();
                    }
                } else {
                    opening_buttons();
                    if (*this->ptr_n_b == *this->ptr_n_m) {
                        if (auto win = QApplication::activeWindow()) win->setEnabled(false);
                            if (restart_game(GameResult::Won) == QMessageBox::Yes) {
                                if (auto win = QApplication::activeWindow()) win->setEnabled(true);
                                this->form_pointer->pushButton->click();
                            } else {
                                QApplication::quit();
                            }
                    }
               }
        }

    }

private:
    void opening_buttons() {
        if (val == 0) {
            --*this->ptr_n_b;
            this->setStyleSheet("background-color: rgb(255, 255, 255)");
            this->setIcon(QIcon());
            this->setText("");
            this->open = true;
            if (this->flag_raised) {
                this->flag_raised = false;
                ++*this->max_flags;
            }
            auto ui = this->form_pointer;

            const auto buttonAt = [ui](int x, int y) {
                if (x < 0 || y < 0) {
                    return static_cast<My_PushButton*>(nullptr);
                }
                auto item = ui->gridLayout_2->itemAtPosition(y, x);
                if (!item) {
                    return static_cast<My_PushButton*>(nullptr);
                }

                auto ret = static_cast<My_PushButton*>(item->widget());
                return !ret || ret->open ? nullptr : ret;
            };

            auto my_x = this->x() / 25;
            auto my_y = this->y() / 25;

            if (auto button = buttonAt(my_x - 1, my_y - 1)) button->opening_buttons();
            if (auto button = buttonAt(my_x, my_y - 1)) button->opening_buttons();
            if (auto button = buttonAt(my_x + 1, my_y - 1)) button->opening_buttons();

            if (auto button = buttonAt(my_x - 1, my_y)) button->opening_buttons();
            if (auto button = buttonAt(my_x + 1, my_y)) button->opening_buttons();

            if (auto button = buttonAt(my_x - 1, my_y + 1)) button->opening_buttons();
            if (auto button = buttonAt(my_x, my_y + 1)) button->opening_buttons();
            if (auto button = buttonAt(my_x + 1, my_y + 1)) button->opening_buttons();
        }
        else if (val != 9){
            --*this->ptr_n_b;
            this->open = true;
            this->setStyleSheet("background-color: rgb(209, 173, 245)");
            this->setIcon(QIcon());
            this->setText(QString::number(val));
        }
    }
signals:
    void rightClicked();
    void leftClicked();
protected:
    void mouseReleaseEvent(QMouseEvent* event) {
        if (event->button() == Qt::RightButton) {
            emit rightClicked();
        }
        else {
            emit leftClicked();
        }
    }
};

#include "mainwindow.moc"

void MainWindow::on_action_triggered(){
    QTextBrowser *tb = new QTextBrowser(); // Create a window with game rules
    tb->setFixedSize(610, 150);
    tb->setText("Use the left click button on the mouse to select a space on the grid.\n"
                "If you hit a bomb, you lose.\n"
                "The numbers on the board represent how many bombs are adjacent to a square.\n"
                "For example, if a square has a \"3\" on it, then there are 3 bombs next to that square.\n"
                "The bombs could be above, below, right and (or) left, or diagonal to the square.\n"
                "Avoid all the bombs and expose all the empty spaces to win Minesweeper.\n"
                "You can right click a square with the mouse to place a flag where you think a bomb is. This allows you to avoid that spot.");
    tb->setEnabled(true);
    tb->setVisible(true);
}

void MainWindow::on_actionNotes_2_triggered()
{
    QTextBrowser *tb = new QTextBrowser(); // Create a window with my notes about this program
    tb->setFixedSize(500, 55);
    tb->setText("One of the older versions of minesweeper is implemented.\n"
                "Mines are generated before you click on a cell, so you can die after the first turn.");
    tb->setEnabled(true);
    tb->setVisible(true);
}

void my_rand(int size, int &numines, Ui::MainWindow *ui) {

    /*************************************************************
     * The following values will be used to customize the cells: *
     * 0 - empty cell;                                           *
     * 1 - cell showing the number of mines around it = 1;       *
     * 2 - cell showing the number of mines around it = 2;       *
     * 3 - cell showing the number of mines around it = 3;       *
     * 4 - cell showing the number of mines around it = 4;       *
     * 5 - cell showing the number of mines around it = 5;       *
     * 6 - cell showing the number of mines around it = 6;       *
     * 7 - cell showing the number of mines around it = 7;       *
     * 8 - cell showing the number of mines around it = 8;       *
     * 9 - mine cell;                                            *
     *************************************************************/


    srand(time(NULL));
    // i & j - the coordinates of the installation of the mine, and in the future - just the coordinates
    int counter = 0, i = 0, j = 0;
    static int temp_size, copy_numines;
    temp_size = size * size; // temp_size used to initialize the pointer and later to complete the game
    copy_numines = numines;

    const auto buttonAt = [ui](int y, int x) {
        return static_cast<My_PushButton*>(ui->gridLayout_2->itemAtPosition(y, x)->widget());
    };

    // Clear the field from old buttons
    if (ui->gridLayout_2->count() != 0) {
        QLayoutItem* item;
        while ((item = ui->gridLayout_2->takeAt(0)) != nullptr) {
            delete item->layout();
            delete item;
        }
    }

    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            ui->gridLayout_2->addWidget(new My_PushButton(), i, j);
            buttonAt(i, j)->form_pointer = ui;
            buttonAt(i, j)->ptr_n_b = &temp_size;
            buttonAt(i, j)->ptr_n_m = &numines;
            buttonAt(i, j)->max_flags = &copy_numines;

        }
    }

    counter = numines;
    while (counter > 0) { //Placing mines
        i = rand() % size, j = rand() % size;
        if (buttonAt(i, j)->val != 9) {
            buttonAt(i, j)->val = 9;
            --counter;
        }
    }

    counter = 0;
    // Checking neighbors for mines
    for (i = 1; i < size - 1; ++i) {                  // Checking an area without borders
        for (j = 1; j < size - 1; ++j) {
            if (buttonAt(i, j)->val != 9) {
                if (buttonAt(i - 1, j - 1)->val == 9) ++counter;
                if (buttonAt(i - 1, j)->val == 9) ++counter;
                if (buttonAt(i - 1, j + 1)->val == 9) ++counter;

                if (buttonAt(i, j - 1)->val == 9) ++counter;
                if (buttonAt(i, j + 1)->val == 9) ++counter;

                if (buttonAt(i + 1, j - 1)->val == 9) ++counter;
                if (buttonAt(i + 1, j)->val == 9) ++counter;
                if (buttonAt(i + 1, j + 1)->val == 9) ++counter;
            }

            if (counter > 0) buttonAt(i, j)->val = counter;
            counter = 0;
        }
    }

    for (i = 0; i < size; ++i) {                   // Upper border
            for (j = 0; j < size; ++j) {
                if (buttonAt(i, j)->val != 9) {
                    if (i == 0 && j == 0) {
                        if (buttonAt(i, j + 1)->val == 9) ++counter;
                        if (buttonAt(i + 1, j)->val == 9) ++counter;
                        if (buttonAt(i + 1, j + 1)->val == 9) ++counter;
                        if (counter > 0) buttonAt(i, j)->val = counter, counter = 0;
                    }
                    if (i == 0 && j == size - 1) {
                        if (buttonAt(i, j - 1)->val == 9) ++counter;
                        if (buttonAt(i + 1, j)->val == 9) ++counter;
                        if (buttonAt(i + 1, j - 1)->val == 9) ++counter;
                        if (counter > 0) buttonAt(i, j)->val = counter, counter = 0;
                    }
                    if (i == 0 && (j > 0 && j < size - 1)) {
                        if (buttonAt(i, j - 1)->val == 9) ++counter;
                        if (buttonAt(i, j + 1)->val == 9) ++counter;
                        if (buttonAt(i + 1, j - 1)->val == 9) ++counter;
                        if (buttonAt(i + 1, j)->val == 9) ++counter;
                        if (buttonAt(i + 1, j + 1)->val == 9) ++counter;
                        if (counter > 0) buttonAt(i, j)->val = counter, counter = 0;
                    }



                    if (i == size - 1 && j == 0) {                    // Lower border
                        if (buttonAt(i - 1, j)->val == 9) ++counter;
                        if (buttonAt(i - 1, j + 1)->val == 9) ++counter;
                        if (buttonAt(i, j + 1)->val == 9) ++counter;
                        if (counter > 0) buttonAt(i, j)->val = counter, counter = 0;
                    }
                    if (i == size - 1 && j == size - 1) {
                        if (buttonAt(i - 1, j)->val == 9) ++counter;
                        if (buttonAt(i - 1, j - 1)->val == 9) ++counter;
                        if (buttonAt(i, j - 1)->val == 9) ++counter;
                        if (counter > 0) buttonAt(i, j)->val = counter, counter = 0;
                    }
                    if (i == size - 1 && (j > 0 && j < size - 1)) {
                        if (buttonAt(i, j - 1)->val == 9) ++counter;
                        if (buttonAt(i - 1, j - 1)->val == 9) ++counter;
                        if (buttonAt(i - 1, j)->val == 9) ++counter;
                        if (buttonAt(i - 1, j + 1)->val == 9) ++counter;
                        if (buttonAt(i, j + 1)->val == 9) ++counter;
                        if (counter > 0) buttonAt(i, j)->val = counter, counter = 0;
                    }



                    if (j == 0 && (i > 0 && i < size - 1)) {        // Left border
                        if (buttonAt(i - 1, j)->val == 9) ++counter;
                        if (buttonAt(i - 1, j + 1)->val == 9) ++counter;
                        if (buttonAt(i, j + 1)->val == 9) ++counter;
                        if (buttonAt(i + 1, j)->val == 9) ++counter;
                        if (buttonAt(i + 1, j + 1)->val == 9) ++counter;
                        if (counter > 0) buttonAt(i, j)->val = counter, counter = 0;
                    }



                    if (j == size - 1 && (i > 0 && i < size - 1)) {    // Right border
                        if (buttonAt(i - 1, j)->val == 9) ++counter;
                        if (buttonAt(i - 1, j - 1)->val == 9) ++counter;
                        if (buttonAt(i, j - 1)->val == 9) ++counter;
                        if (buttonAt(i + 1, j - 1)->val == 9) ++counter;
                        if (buttonAt(i + 1, j)->val == 9) ++counter;
                        if (counter > 0) buttonAt(i, j)->val = counter, counter = 0;
                    }
                }
        }
    }
}

void game_lvl(Ui::MainWindow *ui, int size) {
    static int numines; // Numines is num(ber of m)ines
    numines = (rand() % ((size * size) / 3)) + 1;
    ui->textBrowser->setText(QString::number(numines));
    my_rand(size, numines, ui);
    ui->gridLayout_2->setSpacing(0);
    ui->gridLayout_2->setSizeConstraint(QLayout::SetFixedSize);
}

void MainWindow::on_pushButton_pressed(){
    QVariant index_but = ui->comboBox->currentIndex(); // Find out the value selected in the ComboBox
    int a = index_but.toInt();
    switch (a) {
    case 0:
        game_lvl(ui, 10);
        break;

    case 1:
        game_lvl(ui, 17);
        break;

    case 2:
        game_lvl(ui, 25);
        break;

    default:
        break;
    }
}
