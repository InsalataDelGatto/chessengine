#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
#include <map>
#include <list>
#include <algorithm>
#include <math.h>
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
                colour = White;
                type = code - White;
            }
            else
            {
                colour = Black;
                type = code - Black;
            }
        }
        static int getColour(int code)
        {
            if (code <= 15)
            {
                return White;
            }
            else
            {
                return Black;
            }
        }
};

class Move
{
    public:
        int startingSquare;
        int targetSquare;
        int enPassantSquare;
        int takingSquare;
};

class Board
{
    public:
        int squares[64];
        Texture texture;
        list<Move> legalMoves;
        int enPassantTarget;


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
                    int pieceColour = isupper(symbol) ? Piece::Black : Piece::White;
                    int pieceType = pieceTypeFromSymbol[tolower(symbol)];
                    squares[rank * 8 + file] = pieceType | pieceColour;
                    file++;
                }
            }
        }

        void PlayMove(Move move)
        {
            squares[move.targetSquare] = squares[move.startingSquare];
            if (move.takingSquare != -1)
            {
                squares[move.takingSquare] = 0;
            }
            squares[move.startingSquare] = 0;
            enPassantTarget = move.enPassantSquare;

        }

        void CheckLine(int piecePos, int increment, int range, int takeableColor, Move activeMove)
        {
            int howFarIsNotTooFar = abs(increment) > 1 ? 1 : 0;
            for (int i = 1; i < range + 1; i++)
            {
                if (abs((piecePos + (i * increment)) / 8 - (piecePos + ((i - 1) * increment)) / 8) != howFarIsNotTooFar || piecePos + (i * increment) < 0 || piecePos + (i * increment) > 63)
                {
                    //cout << "tak jsi" << endl;
                    break;
                }
                else if (squares[piecePos + (i * increment)] <= 0)
                {
                    //cout << piecePos + i * increment << endl;
                    activeMove.targetSquare = piecePos + (i * increment);
                    legalMoves.push_back(activeMove);
                }
                else if (Piece::getColour(squares[piecePos + (i * increment)]) == takeableColor)
                {
                    //cout << piecePos + i * increment << endl;
                    activeMove.targetSquare = piecePos + (i * increment);
                    legalMoves.push_back(activeMove);
                    break;
                }
                else
                {
                    //cout << "coco" << endl;
                    break;
                }
            }
        }

        void CheckLegalMoves(int checkedPiecePos)
        {
            cout << "here " << checkedPiecePos << endl;
            Piece checkedPiece;
            checkedPiece.position = checkedPiecePos;
            Move activeMove;
            activeMove.startingSquare = checkedPiece.position;
            activeMove.enPassantSquare = -1;
            activeMove.takingSquare = -1;
            checkedPiece.getTypeAndColour(squares[checkedPiece.position]);
            cout << checkedPiece.type << " " << checkedPiece.colour << endl;
            switch (checkedPiece.type)
            {
                case checkedPiece.Pawn:
                    //if it's a white pawn
                    if (checkedPiece.colour == checkedPiece.White)
                    {
                        //moving by one square
                        if (squares[checkedPiece.position - 8] <= 0)
                        {
                            activeMove.targetSquare = checkedPiece.position - 8;
                            legalMoves.push_back(activeMove);

                            //moving by two squares
                            if ((checkedPiece.position / 8) == 6 && squares[checkedPiece.position - 16] <= 0)
                            {
                                activeMove.targetSquare = checkedPiece.position - 16;
                                activeMove.enPassantSquare = checkedPiece.position - 8;
                                legalMoves.push_back(activeMove);
                            }
                        }

                        //en passant or capture to the left
                        if (checkedPiece.getColour(squares[checkedPiece.position - 7]) == 16)
                        {
                            activeMove.targetSquare = checkedPiece.position - 7;
                            legalMoves.push_back(activeMove);
                        }
                        if (checkedPiece.position - 7 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position - 7;
                            activeMove.takingSquare = checkedPiece.position + 1;
                            legalMoves.push_back(activeMove);
                            activeMove.takingSquare = -1;
                        }

                        //en passant or capture to the right
                        if (checkedPiece.getColour(squares[checkedPiece.position - 9]) == 16)
                        {
                            activeMove.targetSquare = checkedPiece.position - 9;
                            legalMoves.push_back(activeMove);
                        }
                        if (checkedPiece.position - 9 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position - 9;
                            activeMove.takingSquare = checkedPiece.position - 1;
                            legalMoves.push_back(activeMove);
                            activeMove.takingSquare = -1;
                        }
                    }

                    //if it's a black pawn
                    else if (checkedPiece.colour == checkedPiece.Black)
                    {
                        //moving by one square
                        if (squares[checkedPiece.position + 8] <= 0)
                        {
                            activeMove.targetSquare = checkedPiece.position + 8;
                            legalMoves.push_back(activeMove);

                            //moving by two squares
                            if ((checkedPiece.position / 8) == 1 && squares[checkedPiece.position + 16] <= 0)
                            {
                                activeMove.targetSquare = checkedPiece.position + 16;
                                activeMove.enPassantSquare = checkedPiece.position + 8;
                                legalMoves.push_back(activeMove);
                            }
                        }

                        //en passant or capture to the left
                        if (checkedPiece.getColour(squares[checkedPiece.position + 7]) == 8)
                        {
                            activeMove.targetSquare = checkedPiece.position + 7;
                            legalMoves.push_back(activeMove);
                        }
                        if (checkedPiece.position + 7 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position + 7;
                            activeMove.takingSquare = checkedPiece.position + 1;
                            legalMoves.push_back(activeMove);
                            activeMove.takingSquare = -1;
                        }

                        //en passant or capture to the right
                        if (checkedPiece.getColour(squares[checkedPiece.position + 9]) == 8)
                        {
                            activeMove.targetSquare = checkedPiece.position + 9;
                            legalMoves.push_back(activeMove);
                        }
                        if (checkedPiece.position + 9 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position + 9;
                            activeMove.takingSquare = checkedPiece.position - 1;
                            legalMoves.push_back(activeMove);
                            activeMove.takingSquare = -1;
                        }
                    }
                    break;

                //knight
                case checkedPiece.Knight:
                    //white
                    if (checkedPiece.colour == checkedPiece.White)
                    {
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 16 || squares[checkedPiece.position + 6] <= 0) || (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 6;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 10]) == 16 || squares[checkedPiece.position + 10] <= 0) || (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 10;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 15]) == 16 || squares[checkedPiece.position + 15] <= 0) || (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 15;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 17]) == 16 || squares[checkedPiece.position + 17] <= 0) || (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 17;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 6]) == 16 || squares[checkedPiece.position - 6] <= 0) || checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 6;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 10]) == 16 || squares[checkedPiece.position - 10] <= 0) || checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 10;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 15]) == 16 || squares[checkedPiece.position - 15] <= 0) || checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position - 15;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 17]) == 16 || squares[checkedPiece.position - 17] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position - 17;
                            legalMoves.push_back(activeMove);
                        }
                    }
                    //black
                    else if (checkedPiece.colour == checkedPiece.Black)
                    {
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 8 || squares[checkedPiece.position + 6] <= 0) && (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 6;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 10]) == 8 || squares[checkedPiece.position + 10] <= 0) && (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 10;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 15]) == 8 || squares[checkedPiece.position + 15] <= 0) && (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 15;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 17]) == 8 || squares[checkedPiece.position + 17] <= 0) && (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 17;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 6]) == 8 || squares[checkedPiece.position - 6] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 6;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 10]) == 8 || squares[checkedPiece.position - 10] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 10;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 15]) == 8 || squares[checkedPiece.position - 15] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position - 15;
                            legalMoves.push_back(activeMove);
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 17]) == 8 || squares[checkedPiece.position - 17] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position - 17;
                            legalMoves.push_back(activeMove);
                        }
                    }
                    break;

                //king
                case Piece::King:
                    if (checkedPiece.colour == Piece::White)
                    {
                            CheckLine(checkedPiece.position, 1, 1, Piece::Black, activeMove);
                            CheckLine(checkedPiece.position, 7, 1, Piece::Black, activeMove);
                            CheckLine(checkedPiece.position, 8, 1, Piece::Black, activeMove);
                            CheckLine(checkedPiece.position, 9, 1, Piece::Black, activeMove);
                            CheckLine(checkedPiece.position, -1, 1, Piece::Black, activeMove);
                            CheckLine(checkedPiece.position, -7, 1, Piece::Black, activeMove);
                            CheckLine(checkedPiece.position, -8, 1, Piece::Black, activeMove);
                            CheckLine(checkedPiece.position, -9, 1, Piece::Black, activeMove);
                    }
                    if (checkedPiece.colour == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 1, 1, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, 7, 1, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, 8, 1, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, 9, 1, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, -1, 1, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, -7, 1, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, -8, 1, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, -9, 1, Piece::White, activeMove);
                    }
                    break;
                //queen
                case Piece::Queen:
                    if (checkedPiece.colour == Piece::White)
                    {
                        CheckLine(checkedPiece.position, 1, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, 7, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, 8, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, 9, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, -1, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, -7, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, -8, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, -9, 9, Piece::Black, activeMove);
                    }
                    if (checkedPiece.colour == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 1, 9, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, 7, 9, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, 8, 9, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, 9, 9, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, -1, 9, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, -7, 9, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, -8, 9, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, -9, 9, Piece::White, activeMove);
                    }
                    break;
                //Rook
                case Piece::Rook:
                    if (checkedPiece.colour == Piece::White)
                    {
                        CheckLine(checkedPiece.position, 1, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, 8, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, -1, 9, Piece::Black, activeMove);;
                        CheckLine(checkedPiece.position, -8, 9, Piece::Black, activeMove);
                    }
                    if (checkedPiece.colour == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 1, 9, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, 8, 9, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, -1, 9, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, -8, 9, Piece::White, activeMove);

                    }
                    break;
                //Bishop
                case Piece::Bishop:
                    if (checkedPiece.colour == Piece::White)
                    {                       
                        CheckLine(checkedPiece.position, 7, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, 9, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, -7, 9, Piece::Black, activeMove);
                        CheckLine(checkedPiece.position, -9, 9, Piece::Black, activeMove);
                    }
                    if (checkedPiece.colour == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 7, 9, Piece::White, activeMove);
                        CheckLine(checkedPiece.position, 9, 9, Piece::White, activeMove);;
                        CheckLine(checkedPiece.position, -7, 9, Piece::White, activeMove);;
                        CheckLine(checkedPiece.position, -9, 9, Piece::White, activeMove);
                    }
                    break;
            }
        }
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
                        cout << "pozice: " << positionIndex << endl;
                        if (!squareSelected)
                        {
                            if (board.squares[positionIndex] > 0)
                            {
                                board.legalMoves.clear();
                                squareSelected = true;
                                activePiece.position = positionIndex;

                                board.CheckLegalMoves(activePiece.position);
                                for (Move move : board.legalMoves)
                                {
                                    cout << move.targetSquare << " ";                                  
                                }
                                cout << endl;
                            }
                            cout << y << endl << x << endl;
                        }
                        
                        else
                        {
                            for (Move move : board.legalMoves)
                            {
                                if (positionIndex == move.targetSquare)
                                {
                                    board.PlayMove(move);
                                    cout << "played a move!" << endl;
                                }
                            }
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
                int colour = drawnPiece.colour == 8 ? 1 : 0;

                pieceSprite.setTextureRect(IntRect((drawnPiece.type - 1) * 57, (colour) * 60, 57, 60));
                pieceSprite.setPosition(2 + (file * 56), 2 + (rank * 56));
                window.draw(pieceSprite);

                if (i == activePiece.position && squareSelected == true)
                {
                    //cout << "true" << " " << 2 + (file * 56) << " " << 3 + (rank * 56) << endl;
                    selectionSprite.setPosition(2 + (file * 56), 2 + (rank * 56));
                    window.draw(selectionSprite);
                }
            }
        }
        window.display();
    }
    return 0;
}
