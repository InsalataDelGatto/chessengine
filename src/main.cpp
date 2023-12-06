#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
#include <map>
using namespace sf;
using namespace std;

class Piece
{
    public:
        const static int None = 0, King = 5, Pawn = 6, Knight = 2, Bishop = 3, Rook = 1, Queen = 4, White = 8, Black = 16;
};

class Board
{
    public:
        int squares[64];
        Texture texture;
        void LoadPosition(string fen)
        {
            map<char, int>pieceTypeFromSymbol;
            pieceTypeFromSymbol.insert_or_assign('k', Piece::King);
            pieceTypeFromSymbol.insert_or_assign('p', Piece::Pawn);
            pieceTypeFromSymbol.insert_or_assign('n', Piece::Knight);
            pieceTypeFromSymbol.insert_or_assign('b', Piece::Bishop);
            pieceTypeFromSymbol.insert_or_assign('r', Piece::Rook);
            pieceTypeFromSymbol.insert_or_assign('q', Piece::Queen);
            pieceTypeFromSymbol.insert_or_assign('k', Piece::King);
            int file = 0, rank = 7;
            
            for (int square : squares)
            {
                square = 0;
            }

            for (char symbol : fen)
            {
                if (symbol == '/')
                {
                    file = 0;
                    rank--;
                }
                else if (isdigit(symbol))
                {
                    file += symbol + '0';
                }
                else if (symbol == ' ')
                {
                    break;
                }
                else
                {
                    int pieceColour = isupper(symbol) ? Piece::White : Piece::Black;
                    int pieceType = pieceTypeFromSymbol[tolower(symbol)];
                    squares[rank * 8 + file] = pieceType | pieceColour;
                    file++;
                }
            }
        }
};

int main()
{
    Board board;

    board.texture.loadFromFile("images/board0.png");
    Sprite boardSprite;
    boardSprite.setTexture(board.texture);

    Texture pieceTexture;
    pieceTexture.loadFromFile("images/figures.png");
    Sprite pieceSprite;
    pieceSprite.setTexture(pieceTexture);

    board.LoadPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");
    for (int pieceCode : board.squares)
    {
        cout << pieceCode << endl;
    }

    RenderWindow window{VideoMode(453, 453), "Chess"};

    while (window.isOpen())
    {
        Vector2i pos = Mouse::getPosition(window);

        for (auto event = Event{}; window.pollEvent(event);)
        {
            if (event.type == Event::Closed)
            {
                window.close();
            }
        }

        window.clear();
        int colour;
        int type;
        int file;
        int rank;
        window.draw(boardSprite);
        for (int i = 0; i < 64; i++)
        {     
            if (board.squares[i] > 0)
            {
                rank = i / 8;
                file = i % 8;

                if (board.squares[i] < 15)
                {
                    colour = 0;
                    type = board.squares[i] - 9;
                }
                else
                {
                    colour = 1;
                    type = board.squares[i] - 17;
                }
                pieceSprite.setTextureRect(IntRect(type * 57, colour * 60, (type * 57) + 57, (colour * 60) + 60));
                pieceSprite.setPosition(3 + (file * 56), 3 + (rank * 56));
                window.draw(pieceSprite);
            }
        }
        window.display();
    }
    return 0;
}