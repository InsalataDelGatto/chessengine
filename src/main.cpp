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
        inline const static int increments[8] = { 7, 9, -7, -9, 1, 8, -1, -8 };
        int position;
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
        list<Move> kingCapturesWhite;
        list<Move> kingCapturesBlack;
        list<Move> blackPins;
        list<Move> whitePins;

        //new shite
        list<int> blackPieces;
        list<int> whitePieces;

        Board()
        {

        }
        Board(int newSquares[64], list<int> newBlackPieces, list<int> newWhitePieces)
        {
            for (int i = 0; i < 64; i++)
            {
                squares[i] = newSquares[i];
            }
            blackPieces = newBlackPieces;
            whitePieces = newWhitePieces;
        }

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

            for (int i = 0; i < 64; i++)
            {
                if (Piece::getColour(squares[i]) == Piece::White)
                {
                    whitePieces.push_back(i);
                }
                else if (Piece::getColour(squares[i]) == Piece::Black)
                {
                    blackPieces.push_back(i);
                }
            }
        }

        void UpdateBoardInfo()
        {
            int whiteKingPos;
            int blackKingPos;
            whitePieces.clear();
            blackPieces.clear();
            
            //save positions of every piece and king
            for (int i = 0; i < 64; i++)
            {
                if (Piece::getColour(squares[i]) == Piece::White)
                {
                    whitePieces.push_back(i);
                    if (Piece::getType(squares[i]) == Piece::King)
                    {
                        whiteKingPos = i;
                    }
                }
                else if (Piece::getColour(squares[i]) == Piece::Black)
                {
                    blackPieces.push_back(i);
                    if (Piece::getType(squares[i]) == Piece::King)
                    {
                        blackKingPos = i;
                    }
                }
            }

            //check for checks and pins
            for (int piece : whitePieces)
            {
                int pieceType = Piece::getType(squares[piece]);
                CheckLegalMoves(squares[piece], false);
                for (Move &checkedMove : legalMoves)
                {
                    if (checkedMove.targetSquare == blackKingPos)
                    {
                        kingCapturesWhite.push_back(checkedMove);
                    }
                    else if (pieceType != Piece::Knight && pieceType < Piece::King)
                    {
                        for (int increment = pieceType == Piece::Rook ? 3 : 0; increment < pieceType == Piece::Bishop ? 4 : 0; increment++)
                        {
                            for (int rep = 0; rep < 7; rep++)
                            {
                                if (squares[piece + (Piece::increments[increment] * rep)] == Piece::Black)
                                {
                                    break;
                                }
                                else if (squares[piece + (Piece::increments[increment] * rep)] == 0)
                                {
                                    continue;
                                }
                                else if (Piece::getType(squares[piece + (Piece::increments[increment] * rep)]) == Piece::King)
                                {
                                    Move pinMove = checkedMove;
                                    pinMove.targetSquare = piece + (Piece::increments[increment] * rep);
                                    whitePins.push_back(pinMove);
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            for (int piece : blackPieces)
            {
                CheckLegalMoves(squares[piece], false);
                for (Move &checkedMove : legalMoves)
                {
                    if (checkedMove.targetSquare == whiteKingPos)
                    {
                        kingCapturesBlack.push_back(checkedMove);
                    }
                    else if (Piece::getType(piece) != Piece::Knight && Piece::getType(piece) < Piece::King)
                    {
                        for (int increment = Piece::getType(piece) == Piece::Rook ? 3 : 0; increment < Piece::getType(piece) == Piece::Bishop ? 4 : 0; increment++)
                        {
                            for (int rep = 0; rep < 7; rep++)
                            {
                                if (squares[piece + (Piece::increments[increment] * rep)] == Piece::Black)
                                {
                                    break;
                                }
                                else if (squares[piece + (Piece::increments[increment] * rep)] == 0)
                                {
                                    continue;
                                }
                                else if (Piece::getType(squares[piece + (Piece::increments[increment] * rep)]) == Piece::King)
                                {
                                    Move pinMove = checkedMove;
                                    pinMove.targetSquare = piece + (Piece::increments[increment] * rep);
                                    blackPins.push_back(pinMove);
                                }
                                else
                                {
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        void PlayMove(Move move, bool forReal)
        {
            //move the piece, promote if neccessary
            if (move.promoteTo != -1)
            {
                squares[move.targetSquare] = move.promoteTo | Piece::getColour(squares[move.startingSquare]);
            }
            else
            {
                squares[move.targetSquare] = squares[move.startingSquare];
            }
            //take pawn if en passant
            if (move.takingSquare != -1)
            {
                squares[move.takingSquare] = 0;
            }

            //move the rook when castling
            if (move.casLong)
            {
                squares[move.startingSquare - 1] = squares[move.startingSquare - 4];
                squares[move.startingSquare - 4] = 0;
            }
            if (move.casShort)
            {
                squares[move.startingSquare + 1] = squares[move.startingSquare + 3];
                squares[move.startingSquare + 3] = 0;
            }

            //clear the starting square
            squares[move.startingSquare] = 0;

            //update castling rights
            if (move.startingSquare == 0 || move.targetSquare == 0)
            {
                blackLong = false;
            }
            if (move.startingSquare == 4 || move.targetSquare == 4)
            {
                blackLong = false;
                blackShort = false;
            }
            if (move.startingSquare == 7 || move.targetSquare == 7)
            {
                blackShort = false;
            }
            if (move.startingSquare == 56 || move.targetSquare == 56)
            {
                whiteLong = false;
            }
            if (move.startingSquare == 60 || move.targetSquare == 60)
            {
                whiteLong = false;
                whiteShort = false;
            }
            if (move.startingSquare == 63 || move.targetSquare == 63)
            {
                whiteShort = false;
            }

            //if pawn two squares then enable en passant
            enPassantTarget = move.enPassantSquare;

            //keep track of pieces and stuff
            UpdateBoardInfo();
        }

        void CheckLine(int piecePos, int range, int takeableColor, Move activeMove, bool checkIfCheck)
        {
            for (int increment = Piece::getType(squares[piecePos]) == Piece::Rook ? 3 : 0; increment < Piece::getType(squares[piecePos]) == Piece::Bishop ? 4 : 0; increment++)
            {
                int howFarIsNotTooFar = abs(Piece::increments[increment]) > 1 ? 1 : 0;
                for (int i = 1; i < range + 1; i++)
                {
                    if (abs((piecePos + (i * Piece::increments[increment])) / 8 - (piecePos + ((i - 1) * Piece::increments[increment])) / 8) != howFarIsNotTooFar || piecePos + (i * Piece::increments[increment]) < 0 || piecePos + (i * Piece::increments[increment]) > 63)
                    {
                        cout << "tak jsi" << endl;
                        break;
                    }
                    else if (squares[piecePos + (i * Piece::increments[increment])] <= 0)
                    {
                        cout << piecePos + i * Piece::increments[increment] << endl;
                        activeMove.targetSquare = piecePos + (i * Piece::increments[increment]);
                        if (!checkIfCheck)
                        {
                            legalMoves.push_back(activeMove);
                        }
                        else if (IsLegal(activeMove, takeableColor))
                        {
                            legalMoves.push_back(activeMove);
                        }
                    }
                    else if (Piece::getColour(squares[piecePos + (i * Piece::increments[increment])]) == takeableColor)
                    {
                        cout << piecePos + i * Piece::increments[increment] << endl;
                        activeMove.targetSquare = piecePos + (i * Piece::increments[increment]);
                        if (!checkIfCheck)
                        {
                            legalMoves.push_back(activeMove);
                        }
                        else if (IsLegal(activeMove, takeableColor))
                        {
                            legalMoves.push_back(activeMove);
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
        }

        bool IsNotTakeable(int playerToMove, int checkedSquare)
        {
            list<Move> testingMoves;
            if (playerToMove == Piece::White)
            {
                for (int piece : whitePieces)
                {
                    testingMoves.clear();
                    //cout << "checking: " << i << endl;
                    CheckLegalMoves(piece, false);
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
            else
            {
                for (int piece : blackPieces)
                {
                    testingMoves.clear();
                    //cout << "checking: " << i << endl;
                    CheckLegalMoves(piece, false);
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

        bool IsLegal(Move move, int playerToMove)
        {
            Board testBoard;;
            for (int i = 0; i < 64; i++)
            {
                testBoard.squares[i] = squares[i];
            }

            testBoard.PlayMove(move, false);
            if (playerToMove == Piece::White && testBoard.kingCapturesWhite.empty())
            {
                return true;
            }
            else if (playerToMove == Piece::Black && testBoard.kingCapturesBlack.empty())
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        void CheckLegalMoves(int checkedPiecePos, bool checkIfCheck)
        {
            //cout << "here " << checkedPiecePos << endl;
            Piece checkedPiece;
            checkedPiece.position = checkedPiecePos;

            Move activeMove;

            activeMove.casLong = false;
            activeMove.casShort = false;
            activeMove.startingSquare = checkedPiece.position;
            activeMove.enPassantSquare = -1;
            activeMove.takingSquare = -1;
            activeMove.promoteTo = -1;

            //cout << checkedPiece.type << " " << checkedPiece.getColour(squares[checkedPiecePos]) << endl;
            switch (checkedPiece.getType(squares[checkedPiecePos]))
            {
                case checkedPiece.Pawn:
                    //if it's a white pawn
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == checkedPiece.White)
                    {
                        //moving by one square
                        if (squares[checkedPiece.position - 8] <= 0)
                        {
                            activeMove.targetSquare = checkedPiece.position - 8;
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position - 8) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                if ((checkedPiece.position - 8) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }

                            //moving by two squares
                            if ((checkedPiece.position / 8) == 6 && squares[checkedPiece.position - 16] <= 0)
                            {
                                activeMove.targetSquare = checkedPiece.position - 16;
                                activeMove.enPassantSquare = checkedPiece.position - 8;
                                if (!checkIfCheck)
                                {
                                    legalMoves.push_back(activeMove);
                                }
                                else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                        }

                        //en passant or capture to the left
                        if (checkedPiece.getColour(squares[checkedPiece.position - 7]) == 16 && abs(((checkedPiece.position - 7) / 8) - (checkedPiece.position / 8)) == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 7;
                            
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position - 7) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                if ((checkedPiece.position - 7) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                        }
                        if (checkedPiece.position - 7 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position - 7;
                            activeMove.takingSquare = checkedPiece.position + 1;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                            activeMove.takingSquare = -1;
                        }

                        //en passant or capture to the right
                        if (checkedPiece.getColour(squares[checkedPiece.position - 9]) == 16 && abs(((checkedPiece.position - 9) / 8) - (checkedPiece.position / 8)) == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 9;
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position - 9) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                if ((checkedPiece.position - 9) / 8 == 0)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                        }
                        if (checkedPiece.position - 9 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position - 9;
                            activeMove.takingSquare = checkedPiece.position - 1;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                            activeMove.takingSquare = -1;
                        }
                    }

                    //if it's a black pawn
                    else if (checkedPiece.getColour(checkedPiecePos) == checkedPiece.Black)
                    {
                        //moving by one square
                        if (squares[checkedPiece.position + 8] <= 0)
                        {
                            activeMove.targetSquare = checkedPiece.position + 8;
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position + 8) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                if ((checkedPiece.position + 8) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }

                            //moving by two squares
                            if ((checkedPiece.position / 8) == 1 && squares[checkedPiece.position + 16] <= 0)
                            {
                                activeMove.targetSquare = checkedPiece.position + 16;
                                activeMove.enPassantSquare = checkedPiece.position + 8;
                                if (!checkIfCheck)
                                {
                                    legalMoves.push_back(activeMove);
                                }
                                else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                        }

                        //en passant or capture to the left
                        if (checkedPiece.getColour(squares[checkedPiece.position + 7]) == 8)
                        {
                            activeMove.targetSquare = checkedPiece.position + 7;
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position + 7) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                if ((checkedPiece.position + 7) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                        }
                        if (checkedPiece.position + 7 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position + 7;
                            activeMove.takingSquare = checkedPiece.position + 1;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                            activeMove.takingSquare = -1;
                        }

                        //en passant or capture to the right
                        if (checkedPiece.getColour(squares[checkedPiece.position + 9]) == 8)
                        {
                            activeMove.targetSquare = checkedPiece.position + 9;
                            if (!checkIfCheck)
                            {
                                if ((checkedPiece.position + 9) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                if ((checkedPiece.position + 9) / 8 == 7)
                                {
                                    activeMove.promoteTo = Piece::Queen;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Rook;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Bishop;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = Piece::Knight;
                                    legalMoves.push_back(activeMove);
                                    activeMove.promoteTo = -1;
                                }
                                else
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                        }
                        if (checkedPiece.position + 9 == enPassantTarget)
                        {
                            activeMove.targetSquare = checkedPiece.position + 9;
                            activeMove.takingSquare = checkedPiece.position - 1;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                            activeMove.takingSquare = -1;
                        }
                    }
                    break;

                //knight
                case Piece::Knight:
                    //white
                    if (checkedPiece.getColour(checkedPiecePos) == Piece::White)
                    {
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 16 || squares[checkedPiece.position + 6] <= 0) && (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 6;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 10]) == 16 || squares[checkedPiece.position + 10] <= 0) && (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 10;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 15]) == 16 || squares[checkedPiece.position + 15] <= 0) && (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 15;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 17]) == 16 || squares[checkedPiece.position + 17] <= 0) && (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 17;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 6]) == 16 || squares[checkedPiece.position - 6] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1 && checkedPiece.position - 6 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 6;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 10]) == 16 || squares[checkedPiece.position - 10] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1 && checkedPiece.position - 10 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 10;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 15]) == 16 || squares[checkedPiece.position - 15] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2 && checkedPiece.position - 15 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 15;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 17]) == 16 || squares[checkedPiece.position - 17] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2 && checkedPiece.position - 17 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 17;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                    }
                    //black
                    else if (checkedPiece.getColour(checkedPiecePos) == checkedPiece.Black)
                    {
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 6]) == 8 || squares[checkedPiece.position + 6] <= 0) && (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 6;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 10]) == 8 || squares[checkedPiece.position + 10] <= 0) && (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1)
                        {
                            activeMove.targetSquare = checkedPiece.position + 10;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 15]) == 8 || squares[checkedPiece.position + 15] <= 0) && (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 15;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 17]) == 8 || squares[checkedPiece.position + 17] <= 0) && (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2)
                        {
                            activeMove.targetSquare = checkedPiece.position + 17;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 6]) == 8 || squares[checkedPiece.position - 6] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1 && checkedPiece.position - 6 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 6;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 10]) == 8 || squares[checkedPiece.position - 10] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1 && checkedPiece.position - 10 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 10;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 15]) == 8 || squares[checkedPiece.position - 15] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2 && checkedPiece.position - 15 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 15;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 17]) == 8 || squares[checkedPiece.position - 17] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2 && checkedPiece.position - 17 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 17;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == 16 ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                    }
                    break;

                //king
                case Piece::King:
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::White)
                    {
                        CheckLine(checkedPiece.position, 1, Piece::Black, activeMove, checkIfCheck);

                        //castling
                        if (whiteLong && squares[57] + squares[58] + squares[59] == 0 && IsNotTakeable(Piece::Black, 57) && IsNotTakeable(Piece::Black, 58) && IsNotTakeable(Piece::Black, 59) && IsNotTakeable(Piece::Black, 60) && checkedPiece.position == 60)
                        {
                            activeMove.casLong = true;
                            activeMove.targetSquare = 58;
                            legalMoves.push_back(activeMove);
                            activeMove.casLong = false;
                        }
                        if (whiteShort && squares[61] + squares[62] == 0 && IsNotTakeable(Piece::Black, 61) && IsNotTakeable(Piece::Black, 62) && IsNotTakeable(Piece::Black, 60) && checkedPiece.position == 60)
                        {
                            activeMove.casShort = true;
                            activeMove.targetSquare = 62;
                            legalMoves.push_back(activeMove);
                            activeMove.casShort = false;
                        }
                    }
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 1, Piece::White, activeMove,  checkIfCheck);

                        //castling
                        if (blackLong && squares[1] + squares[2] + squares[3] == 0 && IsNotTakeable(Piece::White, 1) && IsNotTakeable(Piece::White, 2) && IsNotTakeable(Piece::White, 3) && IsNotTakeable(Piece::White, 4) && checkedPiece.position == 4)
                        {
                            activeMove.casLong = true;
                            activeMove.targetSquare = 2;
                            legalMoves.push_back(activeMove);
                            activeMove.casLong = false;
                        }
                        if (blackShort && squares[5] + squares[6] == 0 && IsNotTakeable(Piece::White, 5) && IsNotTakeable(Piece::White, 6) && IsNotTakeable(Piece::White, 4) && checkedPiece.position == 4)
                        {
                            activeMove.casShort = true;
                            activeMove.targetSquare = 6;
                            legalMoves.push_back(activeMove);
                            activeMove.casShort = false;
                        }
                    }
                    break;
                //queen
                case Piece::Queen:
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::White)
                    {
                        CheckLine(checkedPiece.position, 9, Piece::Black, activeMove,  checkIfCheck);
                    }
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 9, Piece::White, activeMove,  checkIfCheck);
                    }
                    break;
                //Rook
                case Piece::Rook:
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::White)
                    {
                        CheckLine(checkedPiece.position, 9, Piece::Black, activeMove,  checkIfCheck);
                    }
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 9, Piece::White, activeMove,  checkIfCheck);

                    }
                    break;
                //Bishop
                case Piece::Bishop:
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::White)
                    {                       
                        CheckLine(checkedPiece.position, 9, Piece::Black, activeMove,  checkIfCheck);
                    }
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 9, Piece::White, activeMove, checkIfCheck);
                    }
                    break;
            }
        }
};

class Engine
{
public:
    static int Eval(int board[64])
    {
        int eval = 0;
        for (int i = 0; i < 64; i++)
        {
            int piece = board[i];
            switch (Piece::getType(piece))
            {
            case Piece::Pawn:
                eval += (Piece::getColour(piece) == 16 ? -1 : 1) * 1;
                break;

            case Piece::Knight:
                eval += (Piece::getColour(piece) == 16 ? -1 : 1) * 3;
                break;

            case Piece::Bishop:
                eval += (Piece::getColour(piece) == 16 ? -1 : 1) * 3;
                break;

            case Piece::Rook:
                eval += (Piece::getColour(piece) == 16 ? -1 : 1) * 5;
                break;

            case Piece::Queen:
                eval += (Piece::getColour(piece) == 16 ? -1 : 1) * 9;
                break;

            default:
                break;
            }
        }
        return eval;
    }
    static int minimax(int depth, bool whiteToPlay, int alpha, int beta, Move* bestMove, Board board)
    {
        cout << "bober <3" << endl;
        Board testingBoard{ board.squares, board.blackPieces, board.whitePieces };
        if (depth == 0)
        {
            return Eval(testingBoard.squares);
        }
        
        if (whiteToPlay)
        {
            //cout << "ja pierdole" << endl;
            int maxEval = INT_MIN;
            int kingPos;
            list<int> thisWhitePieces = testingBoard.whitePieces;
            list<Move> placeholderList;
            for (int i : thisWhitePieces)
            {
                if (Piece::getType(testingBoard.squares[i]) == Piece::King)
                {
                    kingPos = i;
                }
                testingBoard.CheckLegalMoves(i, true);
                placeholderList = testingBoard.legalMoves;
                for (Move& testedMove : placeholderList)
                {
                    int startingSquare = testingBoard.squares[testedMove.startingSquare];
                    int targetSquare = testingBoard.squares[testedMove.targetSquare];
                    testingBoard.PlayMove(testedMove, true);
                    int eval = minimax(depth - 1, false, alpha, beta, bestMove, testingBoard);
                    if (eval > maxEval)
                    {
                        maxEval = eval;
                        *bestMove = testedMove;
                    }
                    alpha = fmax(alpha, maxEval);
                    if (beta <= alpha)
                    {
                        break;
                    }
                    testingBoard.squares[testedMove.startingSquare] = startingSquare;
                    testingBoard.squares[testedMove.targetSquare] = targetSquare;
                }
                testingBoard.legalMoves.clear();
            }
            if (maxEval == INT_MIN)
            {
                if (testingBoard.IsNotTakeable(Piece::Black, kingPos))
                {
                    return 0;
                }
                else
                {
                    return INT_MIN;
                }
            }
            
            return maxEval;
        }
        else
        {
            //cout << "kurwa bober" << endl;
            int minEval = INT_MAX;
            int kingPos;
            list<int> thisBlackPieces = testingBoard.blackPieces;
            list<Move> placeholderList;
            cout << testingBoard.blackPieces.size() << endl;
            
            for (int i : thisBlackPieces)
            {
                if (Piece::getType(testingBoard.squares[i]) == Piece::King)
                {
                    kingPos = i;
                }
                cout << "bober <3 <3" << endl;
                testingBoard.CheckLegalMoves(i, true);
                placeholderList = testingBoard.legalMoves;
                for (Move& testedMove : placeholderList)
                {
                    int startingSquare = testingBoard.squares[testedMove.startingSquare];
                    int targetSquare = testingBoard.squares[testedMove.targetSquare];
                    cout << "bober bober bober <3" << endl;
                    testingBoard.PlayMove(testedMove, true);
                    int eval = minimax(depth - 1, true, alpha, beta, bestMove, testingBoard);
                    if (eval < minEval)
                    {
                        minEval = eval;
                        *bestMove = testedMove;
                    }
                    beta = fmin(beta, minEval);
                    if (beta <= alpha)
                    {
                        break;
                    }
                    testingBoard.squares[testedMove.startingSquare] = startingSquare;
                    testingBoard.squares[testedMove.targetSquare] = targetSquare;
                }
                testingBoard.legalMoves.clear();
            }
            if (minEval == INT_MIN)
            {
                if (testingBoard.IsNotTakeable(Piece::White, kingPos))
                {
                    return 0;
                }
                else
                {
                    return INT_MAX;
                }
            }
            
            return minEval;
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
                                        board.PlayMove(move, true);
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
                                        board.PlayMove(move, true);
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
                                        board.PlayMove(move, true);
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
                                        board.PlayMove(move, true);
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

                                board.CheckLegalMoves(activePiece.position, true);
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
                                        board.PlayMove(move, true);
                                        cout << "played a move!" << endl;
                                    }
                                    //activePlayer = activePlayer == Piece::White ? Piece::Black : Piece::White;
                                    activePlayer = Piece::Black;
                                }
                            }
                            squareSelected = false;
                            activePiece.position = -1;
                            cout << "eval: " << Engine::Eval(board.squares) << endl;
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

                int colour = drawnPiece.getColour(board.squares[i]) == 8 ? 1 : 0;

                pieceSprite.setTextureRect(IntRect((drawnPiece.getType(board.squares[i]) - 1) * 57, (colour) * 60, 57, 60));
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
        if (activePlayer == Piece::Black && promotionSquare == -1)
        {
            Move bestMove;
            int eval = Engine::minimax(1, false, INT_MIN, INT_MAX, &bestMove, board);
            board.PlayMove(bestMove, true);
            cout << bestMove.startingSquare << " " << bestMove.targetSquare << endl;
            activePlayer = Piece::White;
        }
    }
    return 0;
}