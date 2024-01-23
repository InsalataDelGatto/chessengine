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
            if (code > 15)
            {
                colour = Black;
                type = code - Black;
            }
            else if (code > 0)
            {
                colour = White;
                type = code - White;
            }
            else
            {
                colour = 0; 
                type = 0;
            }
        }
        static int getColour(int code)
        {
            if (code > 15)
            {
                return Black;
            }
            else if (code > 0)
            {
                return White;
            }
            else
            {
                return 0;
            }
        }
        static int getType(int code)
        {
            if(code > 15)
            {
                return code - Black;
            }
            else if (code > 0)
            {
                return code - White;
            }
            else
            {
                return 0;
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
        bool casLong;
        bool casShort;
        int promoteTo;
};

class Board
{
    public:
        int squares[64];
        Texture texture;
        list<Move> legalMoves;
        int enPassantTarget;
        bool blackLong = true;
        bool blackShort = true;
        bool whiteLong = true;
        bool whiteShort = true;


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

        void PlayMove(Move move, int* squaresPtr, Board* boardPtr, bool forReal)
        {
            if (forReal)
            {
                //tracking castling rules
                if (move.startingSquare == 0 || move.targetSquare == 0)
                {
                    boardPtr->blackLong = false;
                }
                if (move.startingSquare == 4 || move.targetSquare == 4)
                {
                    boardPtr->blackLong = false;
                    boardPtr->blackShort = false;
                }
                if (move.startingSquare == 7 || move.targetSquare == 7)
                {
                    boardPtr->blackShort = false;
                }
                if (move.startingSquare == 56 || move.targetSquare == 56)
                {
                    boardPtr->whiteLong = false;
                }
                if (move.startingSquare == 60 || move.targetSquare == 60)
                {
                    boardPtr->whiteLong = false;
                    boardPtr->whiteShort = false;
                }
                if (move.startingSquare == 63 || move.targetSquare == 63)
                {
                    boardPtr->whiteShort = false;
                }
            }

            //move the piece
            if (move.promoteTo != -1)
            {
                *(squaresPtr + move.targetSquare) = move.promoteTo | Piece::getColour(*(squaresPtr + move.startingSquare));
            }
            else
            {
                *(squaresPtr + move.targetSquare) = *(squaresPtr + move.startingSquare);
            }
            if (move.takingSquare != -1)
            {
                *(squaresPtr + move.takingSquare) = 0;
                cout << "Google en passant!" << endl << "Holy hell" << endl << "New response just dropped" << endl << "Actual zombie" << endl << "Call the exorcist" << endl;
            }

            //move the rook when castling
            if (move.casLong)
            {
                *(squaresPtr + move.startingSquare - 1) = *(squaresPtr + move.startingSquare - 4);
                *(squaresPtr + move.startingSquare - 4) = 0;
                if (*(squaresPtr + move.startingSquare) == 60)
                {
                    boardPtr->whiteLong = false;
                }
                else
                {
                    boardPtr->blackLong = false;
                }
            }
            if (move.casShort)
            {
                *(squaresPtr + move.startingSquare + 1) = *(squaresPtr + move.startingSquare + 3);
                *(squaresPtr + move.startingSquare + 3) = 0;
                if (*(squaresPtr + move.startingSquare) == 60)
                {
                    boardPtr->whiteShort = false;
                }
                else
                {
                    boardPtr->blackShort = false;
                }
            }

            //clear the starting square
            *(squaresPtr + move.startingSquare) = 0;

            //make en passant possible
            boardPtr->enPassantTarget = move.enPassantSquare;
        }

        void CheckLine(int piecePos, int increment, int range, int takeableColor, Move activeMove, list<Move> *moveList, bool checkIfCheck, int moreSquares[64], Board* boardPtr)
        {
            list<Move> *placeholderMoveList = moveList;
            int howFarIsNotTooFar = abs(increment) > 1 ? 1 : 0;
            for (int i = 1; i < range + 1; i++)
            {
                if (abs((piecePos + (i * increment)) / 8 - (piecePos + ((i - 1) * increment)) / 8) != howFarIsNotTooFar || piecePos + (i * increment) < 0 || piecePos + (i * increment) > 63)
                {
                    //cout << "tak jsi" << endl;
                    break;
                }
                else if (moreSquares[piecePos + (i * increment)] <= 0)
                {
                    //cout << piecePos + i * increment << endl;
                    activeMove.targetSquare = piecePos + (i * increment);
                    if (!checkIfCheck)
                    {
                        placeholderMoveList->push_back(activeMove);
                    }
                    else if (IsLegal(activeMove, takeableColor, moreSquares, boardPtr))
                    {
                        placeholderMoveList->push_back(activeMove);
                    }
                }
                else if (Piece::getColour(moreSquares[piecePos + (i * increment)]) == takeableColor)
                {
                    //cout << piecePos + i * increment << endl;
                    activeMove.targetSquare = piecePos + (i * increment);
                    if (!checkIfCheck)
                    {
                        placeholderMoveList->push_back(activeMove);
                    }
                    else if (IsLegal(activeMove, takeableColor, moreSquares, boardPtr))
                    {
                        placeholderMoveList->push_back(activeMove);
                    }
                    break;
                }
                else
                {
                    //cout << "coco" << endl;
                    break;
                }
            }
        }

        bool IsNotTakeable(int playerToMove, int moreSquares[64], int checkedSquare, Board* boardPtr)
        {
            list<Move> testingMoves;

            for (int i = 0; i < 64; i++)
            {
                testingMoves.clear();
                if (Piece::getColour(moreSquares[i]) == playerToMove)
                {
                    //cout << "checking: " << i << endl;
                    CheckLegalMoves(i, &testingMoves, false, moreSquares, boardPtr);
                    for (const Move& testedMove : testingMoves)
                    {
                        //cout << "possible move: " << testedMove.targetSquare << endl;
                        if (testedMove.targetSquare == checkedSquare)
                        {
                            return false;
                        }
                    }
                }
            }
            return true;
        }

        bool IsLegal(Move move, int playerToMove, int moreSquares[64], Board* boardPtr)
        {
            int testSquares[64];
            Board testBoard = *boardPtr;
            list<Move> testingMoves;
            int kingPos = - 1;
            //cout << playerToMove << endl;
            if (Piece::getType(moreSquares[move.startingSquare]) == Piece::King && Piece::getColour(moreSquares[move.startingSquare]) != playerToMove)
            {
                kingPos = move.targetSquare;
                //cout << "king: " << kingPos << endl;
            }

            for (int i = 0; i < 64; i++)
            {
                testSquares[i] = moreSquares[i];
                if (Piece::getColour(testSquares[i]) != playerToMove && testSquares[i] - Piece::getColour(testSquares[i]) == Piece::King && kingPos == -1)
                {
                    kingPos = i;
                }
                //cout << Piece::getColour(testSquares[i]) << " ";
            }

            if (kingPos == -1)
            {
                return false;
            }


            //cout << "king: " << kingPos << endl;
            PlayMove(move, &testSquares[0], &testBoard, false);
            for (int i = 0; i < 64; i++)
            {                
                testingMoves.clear();
                if (Piece::getColour(testSquares[i]) == playerToMove)
                {
                    //cout << "checking: " << i << endl;
                    CheckLegalMoves(i, &testingMoves, false, testSquares, &testBoard);
                    for (const Move& testedMove : testingMoves)
                    {
                        //cout << "possible move: " << testedMove.targetSquare << endl;
                        if (testedMove.targetSquare == kingPos)
                        {
                            return false;
                        }
                    }
                }
            }
            return true;
        }

        void CheckLegalMoves(int checkedPiecePos, list<Move> *finalMoveList, bool checkIfCheck, int moreSquares[64], Board* boardPtr)
        {
            //cout << "here " << checkedPiecePos << endl;
            Piece checkedPiece;
            list<Move>* placeholderMoveList = finalMoveList;
            checkedPiece.position = checkedPiecePos;

            Move activeMove;

            activeMove.casLong = false;
            activeMove.casShort = false;
            activeMove.startingSquare = checkedPiece.position;
            activeMove.enPassantSquare = -1;
            activeMove.takingSquare = -1;
            activeMove.promoteTo = -1;
            checkedPiece.getTypeAndColour(moreSquares[checkedPiece.position]);

            //cout << checkedPiece.type << " " << checkedPiece.colour << endl;
            switch (checkedPiece.type)
            {
                case checkedPiece.Pawn:
                    //if it's a white pawn
                    if (checkedPiece.colour == checkedPiece.White)
                    {
                        //moving by one square
                        if (moreSquares[checkedPiece.position - 8] <= 0)
                        {
                            activeMove.targetSquare = checkedPiece.position - 8;
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position - 8) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                if ((checkedPiece.position - 8) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }

                            //moving by two moreSquares
                            if ((checkedPiece.position / 8) == 6 && moreSquares[checkedPiece.position - 16] <= 0)
                            {
                                activeMove.targetSquare = checkedPiece.position - 16;
                                activeMove.enPassantSquare = checkedPiece.position - 8;
                                if (!checkIfCheck)
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                                else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }
                        }

                        //en passant or capture to the left
                        if (checkedPiece.getColour(moreSquares[checkedPiece.position - 7]) == 16 && abs(((checkedPiece.position - 7) / 8) - (checkedPiece.position / 8)) == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 7;
                            
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position - 7) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                if ((checkedPiece.position - 7) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }
                        }
                        if (checkedPiece.position - 7 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position - 7;
                            activeMove.takingSquare = checkedPiece.position + 1;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            activeMove.takingSquare = -1;
                        }

                        //en passant or capture to the right
                        if (checkedPiece.getColour(moreSquares[checkedPiece.position - 9]) == 16 && abs(((checkedPiece.position - 9) / 8) - (checkedPiece.position / 8)) == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 9;
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position - 9) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                if ((checkedPiece.position - 9) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }
                        }
                        if (checkedPiece.position - 9 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position - 9;
                            activeMove.takingSquare = checkedPiece.position - 1;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            activeMove.takingSquare = -1;
                        }
                    }

                    //if it's a black pawn
                    else if (checkedPiece.colour == checkedPiece.Black)
                    {
                        //moving by one square
                        if (moreSquares[checkedPiece.position + 8] <= 0)
                        {
                            activeMove.targetSquare = checkedPiece.position + 8;
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position + 8) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                if ((checkedPiece.position + 8) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }

                            //moving by two moreSquares
                            if ((checkedPiece.position / 8) == 1 && moreSquares[checkedPiece.position + 16] <= 0)
                            {
                                activeMove.targetSquare = checkedPiece.position + 16;
                                activeMove.enPassantSquare = checkedPiece.position + 8;
                                if (!checkIfCheck)
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                                else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }
                        }

                        //en passant or capture to the left
                        if (checkedPiece.getColour(moreSquares[checkedPiece.position + 7]) == 8)
                        {
                            activeMove.targetSquare = checkedPiece.position + 7;
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position + 7) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                if ((checkedPiece.position + 7) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }
                        }
                        if (checkedPiece.position + 7 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position + 7;
                            activeMove.takingSquare = checkedPiece.position + 1;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            activeMove.takingSquare = -1;
                        }

                        //en passant or capture to the right
                        if (checkedPiece.getColour(moreSquares[checkedPiece.position + 9]) == 8)
                        {
                            activeMove.targetSquare = checkedPiece.position + 9;
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position + 9) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                if ((checkedPiece.position + 9) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    placeholderMoveList->push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    placeholderMoveList->push_back(activeMove);
                                }
                            }
                        }
                        if (checkedPiece.position + 9 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position + 9;
                            activeMove.takingSquare = checkedPiece.position - 1;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            activeMove.takingSquare = -1;
                        }
                    }
                    break;

                //knight
                case checkedPiece.Knight:
                    //white
                    if (checkedPiece.colour == checkedPiece.White)
                    {
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position + 6]) == 16 || moreSquares[checkedPiece.position + 6] <= 0) && (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 6;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position + 10]) == 16 || moreSquares[checkedPiece.position + 10] <= 0) && (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 10;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position + 15]) == 16 || moreSquares[checkedPiece.position + 15] <= 0) && (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 15;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position + 17]) == 16 || moreSquares[checkedPiece.position + 17] <= 0) && (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 17;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position - 6]) == 16 || moreSquares[checkedPiece.position - 6] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1 && checkedPiece.position - 6 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 6;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position - 10]) == 16 || moreSquares[checkedPiece.position - 10] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1 && checkedPiece.position - 10 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 10;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position - 15]) == 16 || moreSquares[checkedPiece.position - 15] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2 && checkedPiece.position - 15 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 15;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position - 17]) == 16 || moreSquares[checkedPiece.position - 17] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2 && checkedPiece.position - 17 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 17;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                    }
                    //black
                    else if (checkedPiece.colour == checkedPiece.Black)
                    {
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position + 6]) == 8 || moreSquares[checkedPiece.position + 6] <= 0) && (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 6;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position + 10]) == 8 || moreSquares[checkedPiece.position + 10] <= 0) && (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 10;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position + 15]) == 8 || moreSquares[checkedPiece.position + 15] <= 0) && (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 15;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position + 17]) == 8 || moreSquares[checkedPiece.position + 17] <= 0) && (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 17;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position - 6]) == 8 || moreSquares[checkedPiece.position - 6] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1 && checkedPiece.position - 6 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 6;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position - 10]) == 8 || moreSquares[checkedPiece.position - 10] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1 && checkedPiece.position - 10 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 10;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position - 15]) == 8 || moreSquares[checkedPiece.position - 15] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2 && checkedPiece.position - 15 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 15;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(moreSquares[checkedPiece.position - 17]) == 8 || moreSquares[checkedPiece.position - 17] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2 && checkedPiece.position - 17 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 17;
                            if (!checkIfCheck)
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.colour == 16 ? 8 : 16, moreSquares, boardPtr))
                            {
                                placeholderMoveList->push_back(activeMove);
                            }
                        }
                    }
                    break;

                //king
                case Piece::King:
                    if (checkedPiece.colour == Piece::White)
                    {
                        CheckLine(checkedPiece.position, 1, 1, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 7, 1, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 8, 1, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 9, 1, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -1, 1, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -7, 1, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -8, 1, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -9, 1, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);

                        //castling
                        if (boardPtr->whiteLong && moreSquares[57] + moreSquares[58] + moreSquares[59] == 0 && IsNotTakeable(Piece::Black, moreSquares, 57, boardPtr) && IsNotTakeable(Piece::Black, moreSquares, 58, boardPtr) && IsNotTakeable(Piece::Black, moreSquares, 59, boardPtr) && IsNotTakeable(Piece::Black, moreSquares, 60, boardPtr) && checkedPiece.position == 60)
                        {
                            activeMove.casLong = true;
                            activeMove.targetSquare = 58;
                            placeholderMoveList->push_back(activeMove);
                            activeMove.casLong = false;
                        }
                        if (boardPtr->whiteShort && moreSquares[61] + moreSquares[62] == 0 && IsNotTakeable(Piece::Black, moreSquares, 61, boardPtr) && IsNotTakeable(Piece::Black, moreSquares, 62, boardPtr) && IsNotTakeable(Piece::Black, moreSquares, 60, boardPtr) && checkedPiece.position == 60)
                        {
                            activeMove.casShort = true;
                            activeMove.targetSquare = 62;
                            placeholderMoveList->push_back(activeMove);
                            activeMove.casShort = false;
                        }
                    }
                    if (checkedPiece.colour == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 1, 1, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 7, 1, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 8, 1, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 9, 1, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -1, 1, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -7, 1, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -8, 1, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -9, 1, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);

                        //castling
                        if (boardPtr->blackLong && moreSquares[1] + moreSquares[2] + moreSquares[3] == 0 && IsNotTakeable(Piece::White, moreSquares, 1, boardPtr) && IsNotTakeable(Piece::White, moreSquares, 2, boardPtr) && IsNotTakeable(Piece::White, moreSquares, 3, boardPtr) && IsNotTakeable(Piece::White, moreSquares, 4, boardPtr) && checkedPiece.position == 4)
                        {
                            activeMove.casLong = true;
                            activeMove.targetSquare = 2;
                            placeholderMoveList->push_back(activeMove);
                            activeMove.casLong = false;
                        }
                        if (boardPtr->blackShort && moreSquares[5] + moreSquares[6] == 0 && IsNotTakeable(Piece::White, moreSquares, 5, boardPtr) && IsNotTakeable(Piece::White, moreSquares, 6, boardPtr) && IsNotTakeable(Piece::White, moreSquares, 4, boardPtr) && checkedPiece.position == 4)
                        {
                            activeMove.casShort = true;
                            activeMove.targetSquare = 6;
                            placeholderMoveList->push_back(activeMove);
                            activeMove.casShort = false;
                        }
                    }
                    break;
                //queen
                case Piece::Queen:
                    if (checkedPiece.colour == Piece::White)
                    {
                        CheckLine(checkedPiece.position, 1, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 7, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 8, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 9, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -1, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -7, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -8, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -9, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                    }
                    if (checkedPiece.colour == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 1, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 7, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 8, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 9, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -1, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -7, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -8, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -9, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                    }
                    break;
                //Rook
                case Piece::Rook:
                    if (checkedPiece.colour == Piece::White)
                    {
                        CheckLine(checkedPiece.position, 1, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 8, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -1, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -8, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                    }
                    if (checkedPiece.colour == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 1, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 8, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -1, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -8, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);

                    }
                    break;
                //Bishop
                case Piece::Bishop:
                    if (checkedPiece.colour == Piece::White)
                    {                       
                        CheckLine(checkedPiece.position, 7, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 9, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -7, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -9, 9, Piece::Black, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                    }
                    if (checkedPiece.colour == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 7, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, 9, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -7, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                        CheckLine(checkedPiece.position, -9, 9, Piece::White, activeMove, placeholderMoveList, checkIfCheck, moreSquares, boardPtr);
                    }
                    break;
            }
        }
};

int main()
{
    Board board;
    bool squareSelected = false;
    int promotingColour = -1;
    int promotionSquare = -1;
    int selectedPieceType;
    Piece activePiece;
    activePiece.position = -1;

    int activePlayer = Piece::White;

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
                        if (promotionSquare > -1)
                        {
                            switch (positionIndex)
                            {
                            case 34:
                                for (Move& move : board.legalMoves)
                                {
                                    if (move.targetSquare == promotionSquare && move.promoteTo == Piece::Rook)
                                    {
                                        board.PlayMove(move, &board.squares[0], &board, true);
                                    }
                                }
                                promotionSquare = -1;
                                promotingColour = -1;
                                break;

                            case 35:
                                for (Move& move : board.legalMoves)
                                {
                                    if (move.targetSquare == promotionSquare && move.promoteTo == Piece::Knight)
                                    {
                                        board.PlayMove(move, &board.squares[0], &board, true);
                                    }
                                }
                                promotionSquare = -1;
                                promotingColour = -1;
                                break;
                                
                            case 36:
                                for (Move& move : board.legalMoves)
                                {
                                    if (move.targetSquare == promotionSquare && move.promoteTo == Piece::Bishop)
                                    {
                                        board.PlayMove(move, &board.squares[0], &board, true);
                                    }
                                }
                                promotionSquare = -1;
                                promotingColour = -1;
                                break;
                                
                            case 37:
                                for (Move& move : board.legalMoves)
                                {
                                    if (move.targetSquare == promotionSquare && move.promoteTo == Piece::Queen)
                                    {
                                        board.PlayMove(move, &board.squares[0], &board, true);
                                    }
                                }
                                promotionSquare = -1;
                                promotingColour = -1;
                                break;

                            default:
                                break;
                            }
                        }
                        else if (!squareSelected)
                        {
                            if (Piece::getColour(board.squares[positionIndex]) == activePlayer)
                            {
                                board.legalMoves.clear();
                                squareSelected = true;
                                activePiece.position = positionIndex;

                                board.CheckLegalMoves(activePiece.position, &board.legalMoves, true, board.squares, &board);
                                cout << endl;
                            }
                            //cout << y << endl << x << endl;
                        }
                        
                        else
                        {
                            for (const Move& move : board.legalMoves)
                            {
                                if (positionIndex == move.targetSquare)
                                {
                                    if (move.promoteTo != -1)
                                    {
                                        promotingColour = Piece::getColour(board.squares[move.startingSquare]);
                                        promotionSquare = move.targetSquare;
                                    }
                                    else
                                    {
                                        board.PlayMove(move, &board.squares[0], &board, true);
                                        cout << "played a move!" << endl;
                                    }
                                    activePlayer = activePlayer == Piece::White ? Piece::Black : Piece::White;
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
                if (promotionSquare > -1)
                {
                    for (int i = 2; i < 6; i++)
                    {
                        pieceSprite.setTextureRect(IntRect((i - 2) * 57, (promotingColour == 8 ? 1 : 0) * 60, 57, 60));
                        pieceSprite.setPosition(2 + (i * 56), 2 + (4 * 56));
                        window.draw(pieceSprite);
                    }
                }
            }
        }
        window.display();
    }
    return 0;
}
