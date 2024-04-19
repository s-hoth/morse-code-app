#include "model.h"
#include <iostream>
#include <fstream>
#include <QFile>
#include <QTextStream>
#include <QTimer>


using namespace std;

Model::Model(QObject *parent) : QObject{parent}
{
    onScreenLetterCounter = 0;
    morseString = "";
    message = "";
    streak = 0;
    fillMorseAlphabetMap();
    level = 1;
    setUpTextfile();
    //QTimer::singleShot(1000, this, [this]() {sendMorse("supercalifragilistic");});
}

void Model::startNewGame()
{
    sendNewWord(); //Temporary, can remove any time just wanted to show it works.
}

void Model::textInputEntered(QString text)
{
    bool correct = true;
    //Receive text and check it against the correct letters.
    QString incorrectString = "You got these wrong: ";
    if(text.length() != message.length())
    {
        correct = false;
        emit toggleCaptain();
        emit sendCaptainText("The message length is incorrect.");
        sendNewWord();
        return;
    }
    for (unsigned char i = 0; i < message.length(); i++)
    {
        if(text[i] != message[i])
        {
            incorrectString.append(text[i]);
            incorrectString.append(", ");
            correct = false;
        }
    }

    if(!correct)
    {
        streak = 0;
        emit updateStreak(streak);
        incorrectString.erase(incorrectString.end()-2, incorrectString.end());
        emit toggleCaptain();
        emit sendCaptainText(incorrectString);
    }
    else
    {
        streak++;
        emit updateStreak(streak);
        emit toggleCaptain();
        emit sendCaptainText("Hooray, you got it right!");
    }

}

void Model::fillMorseAlphabetMap()
{
    // open the file that translates letters into morse
    QFile file(":/assets/morseAlphabet.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    // add the letter from the current line as the key and the morse code as the value
    QTextStream in(&file);
    while (!in.atEnd())
    {
        string line = in.readLine().toStdString();
        MORSE_ALPHABET.insert(pair<char,string>(line[0], line.substr(2)));
    }
}

void Model::sendMorse(string word)
{
    for (char c : word)
    {
        morseString += MORSE_ALPHABET.at(c) + "  ";
    }

    QTimer::singleShot(100, this, &Model::sendMorseHelper);
    emit sendFullMessage(morseString);

}

void Model::sendMorseHelper()
{
    // if there are already 3 letters on screen, clear them before writing new ones
    if (onScreenLetterCounter == 6)
    {
        emit clearMorseBox();
        onScreenLetterCounter = 0;
    }

    // if this is the end of the string, stop writing to the screen.
    if (morseString.size() == 0)
        return;

    // send the first letter of the string to the view, and then remove that letter from the string.
    // tell the view to play the appropriate sound with the dot or dash
    char c = morseString[0];
    morseString = morseString.substr(1);
    if (c == '-')
    {
        emit playDashSound();
        QTimer::singleShot(600, this, &Model::sendMorseHelper);
        emit sendMorseChar("-");
    }
    else if (c == ' ')
    {
        QTimer::singleShot(100, this, &Model::sendMorseHelper);
        emit sendMorseChar(" ");
        onScreenLetterCounter++;
    }
    else if (c == '.')
    {
        emit playDotSound();
        QTimer::singleShot(600, this, &Model::sendMorseHelper);
        emit sendMorseChar("•");
    }
    else
        return;
}

void Model::resetStreak()
{
    streak = 0;
    emit updateStreak(streak);
}

void Model::sendNewWord()
{
    //Pick a random word and display it.
    int random = arc4random() % (currentWords.size() - 1);
    QString word = currentWords.at(random).toLower();
    QTimer::singleShot(1000, this, [word, this]() {sendMorse(word.toStdString());});
}

void Model::setUpTextfile()
{
    QString filename;
    currentWords.clear();

    //TODO: ADD THE PATHS WHEN NEW TEXT FILES ARRIVE.
    if(level == 1)
        filename = ":/assets/levelOneWords";
    else if(level == 2)
        filename = ":/assets/levelTwoWords";
    else
        return;

    QFile file(filename);
    QTextStream stream(&file);

    if(file.open(QIODevice::ReadOnly))
    {
        while(!stream.atEnd())
        {
            QString word = stream.readLine();
            currentWords.push_back(word);
        }
    }
    file.close();
}

