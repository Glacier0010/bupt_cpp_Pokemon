#ifndef POKEMONLIST_H
#define POKEMONLIST_H

#include <QWidget>
#include <QDialog>
#include <QTableView>
#include <QStandardItemModel>

namespace Ui {
class PokemonList;
}

class PokemonList : public QDialog
{
    Q_OBJECT

public:
    explicit PokemonList(const int& uid, const QString& msg, QWidget *parent = 0);
    ~PokemonList();
    Ui::PokemonList *ui;
    QStandardItemModel *poketable;

private:
    const QStringList nameDict = { "Hitmonlee","Primeape","Machop","Chansey","Jigglypuff","Snorlax","Squirtle","Milotic","Slowbro","Pikachu","Charmander","Butterfree" };

};

#endif // POKEMONLIST_H
