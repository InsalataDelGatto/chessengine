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

        Move()
        {
            startingSquare = -1;
            targetSquare = -1;
            enPassantSquare = -1;
            takingSquare = -1;
            casLong = false;
            casShort = false;
            promoteTo = -1;
        }
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
        Board(int newSquares[64], list<int> newBlackPieces, list<int> newWhitePieces, bool newBlackLong, bool newBlackShort, bool newWhiteLong, bool newWhiteShort)
        {
            for (int i = 0; i < 64; i++)
            {
                squares[i] = newSquares[i];
            }
            blackPieces = newBlackPieces;
            whitePieces = newWhitePieces;

            blackLong = newBlackLong;
            blackShort = newBlackShort;
            whiteShort = newWhiteShort;
            whiteLong = newWhiteLong;

            UpdateBoardInfo();
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
                    //cout << symbol - '0' << endl;
                    file += symbol - '0';
                }
                else
                {
                    int pieceColour = isupper(symbol) ? Piece::Black : Piece::White;
                    int pieceType = pieceTypeFromSymbol[tolower(symbol)];
                    //cout << (pieceType | pieceColour) << " rank: " << rank << " file: " << file << endl;
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

        int FindKing(int kingColour)
        {
            if (kingColour == Piece::White)
            {
                for (int piece : whitePieces)
                {
                    if (Piece::getType(squares[piece]) == Piece::King)
                    {
                        return piece;
                    }
                }
            }
            else if (kingColour == Piece::Black)
            {
                for (int piece : blackPieces)
                {
                    if (Piece::getType(squares[piece]) == Piece::King)
                    {
                        return piece;
                    }
                }
            }
            //cout << "no king?" << endl;
            return -1;
        }

        void UpdateBoardInfo()
        {
            int whiteKingPos;
            int blackKingPos;
            whitePieces.clear();
            blackPieces.clear();
            
            //save positions of every piece
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

            whiteKingPos = FindKing(Piece::White);
            blackKingPos = FindKing(Piece::Black);

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
                }
            }
        }

        void PlayMove(Move move)
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
            //anish giri steals pawn
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

            //announce success
            //cout << "zahrano voe" << endl;

            //keep track of pieces and stuff
            UpdateBoardInfo();
        }

        void CheckLine(int piecePos, int range, int takeableColor, bool checkIfCheck)
        {
            Move activeMove;
            activeMove.startingSquare = piecePos;
            for (int increment = (Piece::getType(squares[piecePos]) == Piece::Rook ? 4 : 0); increment < (Piece::getType(squares[piecePos]) == Piece::Bishop ? 4 : 8); increment++)
            {
                int howFarIsNotTooFar = abs(Piece::increments[increment]) > 1 ? 1 : 0;
                for (int i = 1; i <= range; i++)
                {
                    if (abs((piecePos + (i * Piece::increments[increment])) / 8 - (piecePos + ((i - 1) * Piece::increments[increment])) / 8) != howFarIsNotTooFar || piecePos + (i * Piece::increments[increment]) < 0 || piecePos + (i * Piece::increments[increment]) > 63)
                    {
                        //cout << "tak jsi" << endl;
                        break;
                    }
                    else if (squares[piecePos + (i * Piece::increments[increment])] == 0)
                    {
                        //cout << piecePos + i * Piece::increments[increment] << " je prazdny" << endl;
                        activeMove.targetSquare = (piecePos + (i * Piece::increments[increment]));
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
                        //cout << piecePos + i * Piece::increments[increment] << " je zabrany" << endl;
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
            Board testBoard{ squares, blackPieces, whitePieces, blackLong, blackShort, whiteLong, whiteShort };
            if (playerToMove == Piece::White)
            {
                for (int piece : whitePieces)
                {
                    //cout << "checking: " << i << endl;
                    testBoard.CheckLegalMoves(piece, false);
                    for (const Move& testedMove : testBoard.legalMoves)
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
                    //cout << "checking: " << i << endl;
                    testBoard.CheckLegalMoves(piece, false);
                    for (const Move& testedMove : testBoard.legalMoves)
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
            Board testBoard{ squares, blackPieces, whitePieces, blackLong, blackShort, whiteLong, whiteShort };

            testBoard.PlayMove(move);

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
            Piece checkedPiece;
            checkedPiece.position = checkedPiecePos;

            Move activeMove;

            activeMove.casLong = false;
            activeMove.casShort = false;
            activeMove.startingSquare = checkedPiece.position;
            activeMove.enPassantSquare = -1;
            activeMove.takingSquare = -1;
            activeMove.promoteTo = -1;

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
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
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
                                else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                        }

                        //en passant or capture to the left
                        if (checkedPiece.getColour(squares[checkedPiece.position - 7]) == Piece::Black && abs(((checkedPiece.position - 7) / 8) - (checkedPiece.position / 8)) == 1)
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
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
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
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                            activeMove.takingSquare = -1;
                        }

                        //en passant or capture to the right
                        if (checkedPiece.getColour(squares[checkedPiece.position - 9]) == Piece::Black && abs(((checkedPiece.position - 9) / 8) - (checkedPiece.position / 8)) == 1)
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
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
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
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                            activeMove.takingSquare = -1;
                        }
                    }

                    //if it's a black pawn
                    else if (checkedPiece.getColour(squares[checkedPiecePos]) == checkedPiece.Black)
                    {
                        //moving by one square
                        if (checkedPiece.getColour(squares[checkedPiece.position + 8]) == 0)
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
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
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
                            if ((checkedPiece.position / 8) == 1 && checkedPiece.getColour(squares[checkedPiece.position + 16]) == 0)
                            {
                                activeMove.targetSquare = checkedPiece.position + 16;
                                activeMove.enPassantSquare = checkedPiece.position + 8;
                                if (!checkIfCheck)
                                {
                                    legalMoves.push_back(activeMove);
                                }
                                else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                        }

                        //en passant or capture to the left
                        if (checkedPiece.getColour(squares[checkedPiece.position + 7]) == Piece::White && (checkedPiece.position + 7) / 8 - checkedPiece.position / 8 == 1)
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
                            else if (IsLegal(activeMove, Piece::Black))
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
                            activeMove.takingSquare = checkedPiece.position - 1;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, Piece::Black))
                            {
                                legalMoves.push_back(activeMove);
                            }
                            activeMove.takingSquare = -1;
                        }

                        //en passant or capture to the right
                        if (checkedPiece.getColour(squares[checkedPiece.position + 9]) == Piece::White && (checkedPiece.position + 9) / 8 - checkedPiece.position / 8 == 1)
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
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
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
                            activeMove.takingSquare = checkedPiece.position + 1;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
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
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::White)
                    {
                        if (checkedPiece.getColour(squares[checkedPiece.position + 6]) != Piece::White && (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1 && checkedPiece.position + 6 < 64)
                        {
                            activeMove.targetSquare = checkedPiece.position + 6;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if (checkedPiece.getColour(squares[checkedPiece.position + 10]) != Piece::White && (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1 && checkedPiece.position + 10 < 64)
                        {
                            activeMove.targetSquare = checkedPiece.position + 10;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if (checkedPiece.getColour(squares[checkedPiece.position + 15]) != Piece::White && (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2 && checkedPiece.position + 6 < 64)
                        {
                            activeMove.targetSquare = checkedPiece.position + 15;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if (checkedPiece.getColour(squares[checkedPiece.position + 17]) != Piece::White && (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2 && checkedPiece.position + 17 < 64)
                        {
                            activeMove.targetSquare = checkedPiece.position + 17;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if (checkedPiece.getColour(squares[checkedPiece.position - 6]) != Piece::White && checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1 && checkedPiece.position - 6 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 6;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if (checkedPiece.getColour(squares[checkedPiece.position - 10]) != Piece::White && checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1 && checkedPiece.position - 10 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 10;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if (checkedPiece.getColour(squares[checkedPiece.position - 15]) != Piece::White && checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2 && checkedPiece.position - 15 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 15;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if (checkedPiece.getColour(squares[checkedPiece.position - 17]) != Piece::White && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2 && checkedPiece.position - 17 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 17;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                    }
                    //black
                    else if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black)
                    {
                        if (checkedPiece.getColour(squares[checkedPiece.position + 6]) != Piece::Black && (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1 && checkedPiece.position + 6 < 64)
                        {
                            activeMove.targetSquare = checkedPiece.position + 6;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 10]) == Piece::White || squares[checkedPiece.position + 10] <= 0) && (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1 && checkedPiece.position + 10 < 64)
                        {
                            activeMove.targetSquare = checkedPiece.position + 10;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 15]) == Piece::White || squares[checkedPiece.position + 15] <= 0) && (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2 && checkedPiece.position + 15 < 64)
                        {
                            activeMove.targetSquare = checkedPiece.position + 15;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position + 17]) == Piece::White || squares[checkedPiece.position + 17] <= 0) && (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2 && checkedPiece.position + 17 < 64)
                        {
                            activeMove.targetSquare = checkedPiece.position + 17;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 6]) == Piece::White || squares[checkedPiece.position - 6] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1 && checkedPiece.position - 6 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 6;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 10]) == Piece::White || squares[checkedPiece.position - 10] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1 && checkedPiece.position - 10 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 10;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 15]) == Piece::White || squares[checkedPiece.position - 15] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2 && checkedPiece.position - 15 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 15;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
                            {
                                legalMoves.push_back(activeMove);
                            }
                        }
                        if ((checkedPiece.getColour(squares[checkedPiece.position - 17]) == Piece::White || squares[checkedPiece.position - 17] <= 0) && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2 && checkedPiece.position - 17 > -1)
                        {
                            activeMove.targetSquare = checkedPiece.position - 17;
                            if (!checkIfCheck)
                            {
                                legalMoves.push_back(activeMove);
                            }
                            else if (IsLegal(activeMove, checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black ? 8 : 16))
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
                        CheckLine(checkedPiece.position, 1, Piece::Black, checkIfCheck);

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
                        CheckLine(checkedPiece.position, 1, Piece::White, checkIfCheck);

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
                        CheckLine(checkedPiece.position, 8, Piece::Black, checkIfCheck);
                    }
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 8, Piece::White, checkIfCheck);
                    }
                    break;
                //Rook
                case Piece::Rook:
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::White)
                    {
                        CheckLine(checkedPiece.position, 8, Piece::Black, checkIfCheck);
                    }
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 8, Piece::White, checkIfCheck);

                    }
                    break;
                //Bishop
                case Piece::Bishop:
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::White)
                    {                       
                        CheckLine(checkedPiece.position, 8, Piece::Black, checkIfCheck);
                    }
                    if (checkedPiece.getColour(squares[checkedPiecePos]) == Piece::Black)
                    {
                        CheckLine(checkedPiece.position, 8, Piece::White, checkIfCheck);
                    }
                    break;
            }
        }
};

class Engine
{
    inline static const float pawnTable[64] = {
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1.1, 1.2, 1.3, 1.3, 1.2, 1.1, 1,
        1, 1.1, 1.2, 1.4, 1.4, 1.2, 1.1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
    };

    inline static const float knightTable[64] = {
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1.1, 1.2, 1.3, 1.3, 1.2, 1.1, 1,
        1, 1.1, 1.2, 1.4, 1.4, 1.2, 1.1, 1,
        1, 1.1, 1.2, 1.4, 1.4, 1.2, 1.1, 1,
        1, 1.1, 1.2, 1.3, 1.3, 1.2, 1.1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1,
    };

    inline static const float kingTable[64] = {
        0.3, 0.4, 0.3, 0.2, 0.2, 0.3, 0.4, 0.3,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0,
    };

public:
    static const int maxDepth = 0;

    static float Eval(Board board)
    {
        float eval = 0;
        for (int pos : board.blackPieces)
        {
            int piece = board.squares[pos];
            switch (Piece::getType(piece))
            {
            case Piece::Pawn:
                eval -= 1 * pawnTable[pos];
                break;

            case Piece::Knight:
                eval -= 3 * knightTable[pos];
                break;

            case Piece::Bishop:
                eval -= 3;
                break;

            case Piece::Rook:
                eval -= 5;
                break;

            case Piece::Queen:
                eval -= 9;
                break;

            case Piece::King:
                eval -= kingTable[pos];
                break;

            default:
                break;
            }
        }
        for (int pos : board.whitePieces)
        {
            int piece = board.squares[pos];
            switch (Piece::getType(piece))
            {
            case Piece::Pawn:
                eval += 1 * pawnTable[63 - pos];
                break;

            case Piece::Knight:
                eval += 3 * knightTable[63 - pos];
                break;

            case Piece::Bishop:
                eval += 3;
                break;

            case Piece::Rook:
                eval += 5;
                break;

            case Piece::Queen:
                eval += 9;
                break;

            case Piece::King:
                eval += kingTable[63 - pos];
                break;

            default:
                break;
            }
        }
        //cout << "eval: " << eval << endl;
        return eval;
    }
    static float minimax(int depth, bool whiteToPlay, int alpha, int beta, Board board)
    {
        board.UpdateBoardInfo();

        //cout << "depth: " << depth << endl;

        //reached max search depth, return the evaluation
        if (depth == 0)
        {
            return Eval(board);
        }
        
        //not max depth yet, check possible moves
        if (whiteToPlay)
        {
            //eval starts at the minimum
            float maxEval = INT_MIN;

            for (int piece : board.whitePieces) //using the input board here so the list doesn't change during the loop
            {
                cout << "white piece: " << piece << endl;
                
                board.CheckLegalMoves(piece, true);
                for (Move& testedMove : board.legalMoves)
                {
                    Board testingBoard{ board.squares, board.blackPieces, board.whitePieces, board.blackLong, board.blackShort, board.whiteLong, board.whiteShort };
                    testingBoard.PlayMove(testedMove);
                    float eval = minimax(depth - 1, false, alpha, beta, testingBoard);
                    if (eval > maxEval)
                    {
                        maxEval = eval;
                    }
                    alpha = fmax(alpha, maxEval);
                    if (beta <= alpha)
                    {
                        break;
                    }
                }
            }
            if (maxEval == INT_MIN)
            {
                if (board.IsNotTakeable(Piece::Black, board.FindKing(Piece::White)))
                {
                    return 0;
                }
                else
                {
                    return INT_MIN;
                }
            }
            
            //cout << maxEval << endl;
            return maxEval;
        }
        else
        {            
            float minEval = INT_MAX;
            
            for (int piece : board.blackPieces) //using the input board here so the list doesn't change during the loop
            {
                cout << "black piece: " << piece << endl;

                //cout << "piece: " << piece << endl;                
                board.CheckLegalMoves(piece, true);
                for (Move& testedMove : board.legalMoves)
                {
                    Board testingBoard{ board.squares, board.blackPieces, board.whitePieces, board.blackLong, board.blackShort, board.whiteLong, board.whiteShort };
                    //cout << "target: " << testedMove.targetSquare << endl;
                    testingBoard.PlayMove(testedMove);
                    float eval = minimax(depth - 1, true, alpha, beta, testingBoard);
                    if (eval < minEval)
                    {
                        minEval = eval;
                    }
                    beta = fmin(beta, minEval);
                    if (beta <= alpha)
                    {
                        break;
                    }
                }
            }
            if (minEval == INT_MAX)
            {
                if (board.IsNotTakeable(Piece::White, board.FindKing(Piece::White)))
                {
                    return 0;
                }
                else
                {
                    return INT_MAX;
                }
            }
            
            //cout << minEval << endl;
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

    
    cout << endl;

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
                        //cout << "pozice: " << positionIndex << endl;
                        if (promotionSquare > -1)
                        {
                            switch (positionIndex)
                            {
                            case 34:
                                for (Move& move : board.legalMoves)
                                {
                                    if (move.targetSquare == promotionSquare && move.promoteTo == Piece::Rook)
                                    {
                                        board.PlayMove(move);
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
                                        board.PlayMove(move);
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
                                        board.PlayMove(move);
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
                                        board.PlayMove(move);
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
                                //cout << endl;
                                if (board.legalMoves.empty())
                                {
                                    for (int piece : board.whitePieces)
                                    {
                                        board.CheckLegalMoves(piece, true);
                                    }
                                    if (board.legalMoves.empty())
                                    {
                                        cout << "game fucking ended bruv" << endl;
                                        window.close();
                                        return 0;
                                    }
                                    board.legalMoves.clear();
                                }
                            }
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
                                        board.PlayMove(move);
                                        //cout << "played a move!" << endl;
                                    }                                    
                                    activePlayer = Piece::Black;
                                }
                            }
                            squareSelected = false;
                            activePiece.position = -1;
                            promotionSquare = -1;
                            Engine::Eval(board);
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

                int colour = drawnPiece.getColour(board.squares[i]) == Piece::White ? 1 : 0;

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
                        pieceSprite.setTextureRect(IntRect((i - 2) * 57, (promotingColour == Piece::White ? 1 : 0) * 60, 57, 60));
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
            float minEval = INT_MAX;

            board.legalMoves.clear();
            for (int piece : board.blackPieces)
            {
                //cout << "piece: " << piece << endl;
                board.CheckLegalMoves(piece, true);
            }
            for (Move& move : board.legalMoves)
            {
                Board testingBoard{ board.squares, board.blackPieces, board.whitePieces, board.blackLong, board.blackShort, board.whiteLong, board.whiteShort };
                //cout << "Move: " << move.startingSquare << " " << move.targetSquare << endl;
                testingBoard.PlayMove(move);
                float eval = Engine::minimax(Engine::maxDepth, true, INT_MIN, INT_MAX, testingBoard);
                if (eval < minEval)
                {
                    bestMove = move;
                    minEval = eval;
                }
            }
            if (bestMove.startingSquare == -1 || bestMove.targetSquare == -1)
            {
                cout << "game fucking ended bruv" << endl;
                window.close();
                return 0;
            }
            board.PlayMove(bestMove);
            //cout << bestMove.startingSquare << " " << bestMove.targetSquare << endl;
            activePlayer = Piece::White;
        }
    }
    return 0;
}