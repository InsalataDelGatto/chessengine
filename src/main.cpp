#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
#include <map>
#include <list>
#include <algorithm>
using namespace sf;
using namespace std;

class Piece
{
    public:
        const static int None = 0, King = 5, Pawn = 6, Knight = 2, Bishop = 3, Rook = 1, Queen = 4, White = 8, Black = 16;
        int position;
        int type;
        int colour;
        void getTypeAndColour(int code)
        {
            if (code < 15)
            {
                colour = 0;
                type = code - 9;
            }
            else
            {
                colour = 1;
                type = code - 17;
            }
        }
        int getColour(int code)
        {
            if (code < 15)
            {
                return 8;
            }
            else
            {
                return 16;
            }
        }
};

class Move
{
    public:
        int startingSquare;
        int targetSquare;
};

class Board
{
    public:
        int squares[64];
        Texture texture;
        list<Move> legalMoves;
        list<int> enPassantTargets;


        //Loads a position from a fen code
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
            
            for (int i = 0; i < 64; i++)
            {
                squares[i] = 0;
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
                else
                {
                    int pieceColour = isupper(symbol) ? Piece::White : Piece::Black;
                    int pieceType = pieceTypeFromSymbol[tolower(symbol)];
                    squares[rank * 8 + file] = pieceType | pieceColour;
                    file++;
                }
            }
        }

        /*
        void CheckSquare(int indexModifier, int targetValue)
        {
            if (squares[index + indexModifier] <= targetValue)
            {
                activeMove.targetSquare.x = (index + indexModifier) % 8;
                activeMove.targetSquare.y = (index + indexModifier) / 8;
                legalMoves.push_back(activeMove);
            }
        }
        */

        /*void CheckLegalMoves(Piece checkedPiece)
        {
            Move activeMove;
            activeMove.startingSquare = checkedPiece.position;
            checkedPiece.getTypeAndColour(squares[checkedPiece.position]);
            switch (checkedPiece.type)
            {
                case 1:
                    //if it's a white pawn
                    if (checkedPiece.colour == 8)
                    {
                        //moving by one square
                        if (squares[checkedPiece.position + 8] <= 0)
                        {
                            activeMove.targetSquare.position = checkedPiece.position + 8;
                            legalMoves.push_back(activeMove);

                            //moving by two squares
                            if ((checkedPiece.position / 8) == 2 && squares[checkedPiece.position + 16] <= 0)
                            {
                                activeMove.targetSquare.position = checkedPiece.position + 16;
                                legalMoves.push_back(activeMove);
                            }
                        }

                        //en passant or capture to the left
                        if (find(enPassantTargets.begin, enPassantTargets.end, checkedPiece.position + 7) != enPassantTargets.end || checkedPiece.getColour(squares[checkedPiece.position + 7]) == 16)
                        {
                            activeMove.targetSquare.position = checkedPiece.position + 7;
                        }

                        //en passant or capture to the right
                        if (find(enPassantTargets.begin, enPassantTargets.end, checkedPiece.position + 9) != enPassantTargets.end || checkedPiece.getColour(squares[checkedPiece.position + 9]) == 16)
                        {
                            activeMove.targetSquare.position = checkedPiece.position + 9;
                        }
                    }

                    //if it's a black pawn
                    else if (checkedPiece.colour == 16)
                    {
                        //moving by one square
                        if (squares[checkedPiece.position - 8] <= 0)
                        {
                            activeMove.targetSquare.position = checkedPiece.position - 8;
                            legalMoves.push_back(activeMove);

                            //moving by two squares
                            if ((checkedPiece.position / 8) == 2 && squares[checkedPiece.position - 16] <= 0)
                            {
                                activeMove.targetSquare.position = checkedPiece.position - 16;
                                legalMoves.push_back(activeMove);
                            }
                        }

                        //en passant or capture to the left
                        if (find(enPassantTargets.begin, enPassantTargets.end, checkedPiece.position - 7) != enPassantTargets.end || checkedPiece.getColour(squares[checkedPiece.position - 7]) == 8)
                        {
                            activeMove.targetSquare.position = checkedPiece.position - 7;
                            legalMoves.push_back(activeMove);
                        }

                        //en passant or capture to the right
                        if (find(enPassantTargets.begin, enPassantTargets.end, checkedPiece.position - 9) != enPassantTargets.end || checkedPiece.getColour(squares[checkedPiece.position - 9]) == 8)
                        {
                            activeMove.targetSquare.position = checkedPiece.position - 9;
                            legalMoves.push_back(activeMove);
                        }
                    }

                //knight
                case 2:
                    //white
                    if (checkedPiece.colour == 8)
                    {
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 16 || squares[checkedPiece.position + 6] <= 0) && (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare.position = checkedPiece.position + 6;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 16 || squares[checkedPiece.position + 10] <= 0) && (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare.position = checkedPiece.position + 10;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 16 || squares[checkedPiece.position + 15] <= 0) && (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare.position = checkedPiece.position + 15;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 16 || squares[checkedPiece.position + 17] <= 0) && (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare.position = checkedPiece.position + 17;
                            legalMoves.push_back(activeMove);
                        }if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 16 || squares[checkedPiece.position - 6] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1)
                        {
                            activeMove.targetSquare.position = checkedPiece.position - 6;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 16 || squares[checkedPiece.position - 10] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1)
                        {
                            activeMove.targetSquare.position = checkedPiece.position - 10;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 16 || squares[checkedPiece.position - 15] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2)
                        {
                            activeMove.targetSquare.position = checkedPiece.position - 15;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 16 || squares[checkedPiece.position - 17] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2)
                        {
                            activeMove.targetSquare.position = checkedPiece.position - 17;
                            legalMoves.push_back(activeMove);
                        }
                    }
                    //black
                    else if (checkedPiece.colour == 16)
                    {
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 8 || squares[checkedPiece.position + 6] <= 0) && (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare.position = checkedPiece.position + 6;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 8 || squares[checkedPiece.position + 10] <= 0) && (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare.position = checkedPiece.position + 10;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 8 || squares[checkedPiece.position + 15] <= 0) && (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare.position = checkedPiece.position + 15;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 8 || squares[checkedPiece.position + 17] <= 0) && (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare.position = checkedPiece.position + 17;
                            legalMoves.push_back(activeMove);
                        }if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 8 || squares[checkedPiece.position - 6] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1)
                        {
                            activeMove.targetSquare.position = checkedPiece.position - 6;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 8 || squares[checkedPiece.position - 10] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1)
                        {
                            activeMove.targetSquare.position = checkedPiece.position - 10;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 8 || squares[checkedPiece.position - 15] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2)
                        {
                            activeMove.targetSquare.position = checkedPiece.position - 15;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 8 || squares[checkedPiece.position - 17] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2)
                        {
                            activeMove.targetSquare.position = checkedPiece.position - 17;
                            legalMoves.push_back(activeMove);
                        }
                    }

            }
        }*/
};

int main()
{
    Board board;
    bool squareSelected = false;
    int selectedPieceType;
    Piece activePiece;
    activePiece.position = -1;

    board.texture.loadFromFile("images/board0.png");
    Sprite boardSprite;
    boardSprite.setTexture(board.texture);

    Texture pieceTexture;
    pieceTexture.loadFromFile("images/figures.png");
    Sprite pieceSprite;
    pieceSprite.setTexture(pieceTexture);

    Texture selectionTexture;
    selectionTexture.loadFromFile("images/selection.png");
    Sprite selectionSprite;
    selectionSprite.setTexture(selectionTexture, true);

    board.LoadPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    RenderWindow window{VideoMode(453, 453), "Chess"};

    while (window.isOpen())
    {
        Event event;

        Vector2i pos = Mouse::getPosition(window);

        while (window.pollEvent(event))
        {
            // check the type of the event...
            switch (event.type)
            {
                // window closed
                case sf::Event::Closed:
                    window.close();
                    break;

                // key pressed
                case sf::Event::MouseButtonPressed:            
                    if (event.mouseButton.button == Mouse::Left)
                    {
                        int x = (event.mouseButton.x - 3) / 56;
                        int y = (event.mouseButton.y - 3) / 56;
                        int positionIndex = (8 * y) + x;
                        if (!squareSelected)
                        {
                            if (board.squares[positionIndex] > 0)
                            {
                                squareSelected = true;
                                activePiece.position = positionIndex;

                                //board.CheckLegalMoves(activePiece);
                            }
                            cout << y << endl << x << endl;
                        }
                        
                        /*else if (event.mouseCoordinates is in board.legalMoves[]);
                        {
                            //play move
                            squareSelected = false;
                        }*/
                        
                        else
                        {
                            squareSelected = false;
                            activePiece.position = -1;
                            selectedPieceType = 0;
                        }
                    }
                    break;

                // we don't process other types of events
                default:
                    break;
            }
        }

        window.clear();
        Piece drawnPiece;
        int file;
        int rank;
        window.draw(boardSprite);
        for (int i = 0; i < 64; i++)
        {     
            if (board.squares[i] > 0)
            {
                rank = i / 8;
                file = i % 8;

                drawnPiece.getTypeAndColour(board.squares[i]);
                pieceSprite.setTextureRect(IntRect(drawnPiece.type * 57, drawnPiece.colour * 60, 57, 60));
                pieceSprite.setPosition(2 + (file * 56), 2 + (rank * 56));
                window.draw(pieceSprite);

                if (i == activePiece.position && squareSelected == true)
                {
                    cout << "true" << " " << 2 + (file * 56) << " " << 3 + (rank * 56) << endl;
                    selectionSprite.setPosition(2 + (file * 56), 2 + (rank * 56));
                    window.draw(selectionSprite);
                }
            }
        }
        window.display();
    }
    return 0;
}
