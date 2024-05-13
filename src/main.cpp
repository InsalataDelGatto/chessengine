#include <SFML/Graphics.hpp>
#include <time.h>
#include <iostream>
#include <map>
#include <list>
#include <algorithm>
#include <math.h>
using namespace sf;
using namespace std;

/*
public void CrashPrevention()
{
    if (gonnaCrash)
    {
        dont;
    }
}
*/

class Piece
{
    public:
        const static int None = 0, King = 5, Pawn = 6, Knight = 2, Bishop = 3, Rook = 1, Queen = 4, White = 8, Black = 16;
        inline const static int increments[8] = { 7, 9, -7, -9, 1, 8, -1, -8 };
        int position;

        Piece()
        {
            position = -1;
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
        vector<int> squares;
        Texture texture;

        list<Move> legalMoves;
        list<Move> kingCapturesWhite;
        list<Move> kingCapturesBlack;
        list<Move> blackPins;

        list<Move> whitePins;
        list<int> blackPieces;
        list<int> whitePieces;

        list<int> whitePawns;
        list<int> blackPawns;

        bool whiteInCheck = false;
        bool blackInCheck = false;
        bool giveFeedback = false;

        bool endgame = false;

        Board()
        {
            enPassantTarget = -1;
            for (int i = 0; i < 64; i++)
            {
                squares.push_back(0);
            }
        }
        Board(vector<int> newSquares, list<int> newBlackPieces, list<int> newWhitePieces, bool newBlackLong, bool newBlackShort, bool newWhiteLong, bool newWhiteShort)
        {
            for (int i = 0; i < 64; i++)
            {
                squares.push_back(0);
            }
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
                squares.push_back(0);
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
                    file += symbol - '0';
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
            return -1;
        }

        void UpdateBoardInfo()
        {
            int whiteKingPos;
            int blackKingPos;

            whiteInCheck = false;
            blackInCheck = false;

            whitePieces.clear();
            blackPieces.clear();

            whitePawns.clear();
            blackPawns.clear();
            
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

            legalMoves.clear();

            //check for checks and pins
            for (int piece : whitePieces)
            {
                if (giveFeedback)
                {
                    //cout << "white piece: " << piece << endl;
                }
                int pieceType = Piece::getType(squares[piece]);
                CheckLegalMoves(piece, false);
                for (Move &checkedMove : legalMoves)
                {
                    if (checkedMove.targetSquare == blackKingPos)
                    {
                        kingCapturesWhite.push_back(checkedMove);
                        blackInCheck = true;
                        if (giveFeedback)
                        {
                            //cout << "black in check" << endl;
                            //cout << "checking move: " << checkedMove.startingSquare << " " << checkedMove.targetSquare << endl;
                        }
                    }
                    
                }
            }

            legalMoves.clear();

            for (int piece : blackPieces)
            {   
                if (giveFeedback)
                {
                    //cout << "black piece: " << piece << endl;
                }
                CheckLegalMoves(squares[piece], false);
                for (Move &checkedMove : legalMoves)
                {
                    if (checkedMove.targetSquare == whiteKingPos)
                    {
                        kingCapturesBlack.push_back(checkedMove);
                        whiteInCheck = true;
                        //cout << "white in check, stupid!" << endl;
                    }
                }
            }
        }

        void PlayMove(Move move, bool update)
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
                cout << "HOLY HELL!" << endl;
                cout << "NEW RESPONSE JUST DROPPED!" << endl;
                cout << "ACTUAL ZOMBIE" << endl;
                cout << "CALL THE EXORCIST" << endl;
                cout << "BISHOP GOES ON VACATION, NEVER COMES BACK" << endl;
                cout << "QUEEN SACRIFICE, ANYONE?" << endl;
                cout << "PAWN STORM INCOMING" << endl;
                cout << "KNIGHTMARE FUEL" << endl;
                cout << "CHEKMATE OR RIOT" << endl;
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
            //cout << "played" << endl;

            //keep track of pieces and stuff
            if (update)
            {
                UpdateBoardInfo();
            }
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
                        break;
                    }
                    else if (squares[piecePos + (i * Piece::increments[increment])] == 0)
                    {
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
                    testBoard.CheckLegalMoves(piece, false);
                    for (const Move& testedMove : testBoard.legalMoves)
                    {
                        if (testedMove.targetSquare == checkedSquare)
                        {
                            return false;
                        }
                    }
                    testBoard.legalMoves.clear();
                }
            }
            else
            {
                for (int piece : blackPieces)
                {
                    testBoard.CheckLegalMoves(piece, false);
                    for (const Move& testedMove : testBoard.legalMoves)
                    {
                        if (testedMove.targetSquare == checkedSquare)
                        {
                            return false;
                        }
                    }
                    testBoard.legalMoves.clear();
                }
            }
            return true;
        }

        bool IsLegal(Move move, int playerToMove)
        {
            Board testingBoard{ squares, blackPieces, whitePieces, blackLong, blackShort, whiteLong, whiteShort };

            if (playerToMove == Piece::Black)
            {
                testingBoard.PlayMove(move, true);
                return !testingBoard.whiteInCheck;
            }
            if (playerToMove == Piece::White)
            {
                testingBoard.PlayMove(move, true);
                return !testingBoard.blackInCheck;
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
                        if ((checkedPiece.position / 8) != 0 && squares[checkedPiece.position - 8] == 0)
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
                                else if ((checkedPiece.position - 8) / 8 >= 0)
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

                        //en passant or capture to the right
                        if ((checkedPiece.position / 8) != 0 && checkedPiece.position % 8 != 7 && checkedPiece.getColour(squares[checkedPiece.position - 7]) == Piece::Black)
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
                                else if ((checkedPiece.position - 7) / 8 >= 0)
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
                                else if ((checkedPiece.position - 7) / 8 >= 0)
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                        }
                        if ((checkedPiece.position / 8) != 0 && checkedPiece.position % 8 != 7 && checkedPiece.position - 7 == enPassantTarget)
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

                        //en passant or capture to the left
                        if ((checkedPiece.position / 8) != 0 && checkedPiece.position % 8 != 0 && checkedPiece.getColour(squares[checkedPiece.position - 9]) == Piece::Black)
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
                                else if ((checkedPiece.position - 9) / 8 >= 0)
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
                                else if ((checkedPiece.position - 9) / 8 >= 0)
                                {
                                    legalMoves.push_back(activeMove);
                                }
                            }
                        }
                        if ((checkedPiece.position / 8) != 0 && checkedPiece.position % 8 != 0 && checkedPiece.position - 9 == enPassantTarget)
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
                            else if (IsLegal(activeMove, Piece::White))
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
                            else if (IsLegal(activeMove, Piece::White))
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
                        if (checkedPiece.position + 6 < 64 && checkedPiece.getColour(squares[checkedPiece.position + 6]) != Piece::White && (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1)
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
                        if (checkedPiece.position + 10 < 64 && checkedPiece.getColour(squares[checkedPiece.position + 10]) != Piece::White && (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1)
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
                        if (checkedPiece.position + 15 < 64 && checkedPiece.getColour(squares[checkedPiece.position + 15]) != Piece::White && (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2)
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
                        if (checkedPiece.position + 17 < 64 && checkedPiece.getColour(squares[checkedPiece.position + 17]) != Piece::White && (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2)
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
                        if (checkedPiece.position - 6 > -1 && checkedPiece.getColour(squares[checkedPiece.position - 6]) != Piece::White && checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1)
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
                        if (checkedPiece.position - 10 > -1 && checkedPiece.getColour(squares[checkedPiece.position - 10]) != Piece::White && checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1)
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
                        if (checkedPiece.position - 15 > -1 && checkedPiece.getColour(squares[checkedPiece.position - 15]) != Piece::White && checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2)
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
                        if (checkedPiece.position - 17 > -1 && checkedPiece.getColour(squares[checkedPiece.position - 17]) != Piece::White && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2)
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
                        if (checkedPiece.position + 6 < 64 && checkedPiece.getColour(squares[checkedPiece.position + 6]) != Piece::Black && (checkedPiece.position + 6) / 8 - checkedPiece.position / 8 == 1)
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
                        if (checkedPiece.position + 10 < 64 && checkedPiece.getColour(squares[checkedPiece.position + 10]) != Piece::Black && (checkedPiece.position + 10) / 8 - checkedPiece.position / 8 == 1)
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
                        if (checkedPiece.position + 15 < 64 && checkedPiece.getColour(squares[checkedPiece.position + 15]) != Piece::Black && (checkedPiece.position + 15) / 8 - checkedPiece.position / 8 == 2)
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
                        if (checkedPiece.position + 17 < 64 && checkedPiece.getColour(squares[checkedPiece.position + 17]) != Piece::Black && (checkedPiece.position + 17) / 8 - checkedPiece.position / 8 == 2)
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
                        if (checkedPiece.position - 6 >= 0 && checkedPiece.getColour(squares[checkedPiece.position - 6]) != Piece::Black && checkedPiece.position / 8 - (checkedPiece.position - 6) / 8 == 1)
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
                        if (checkedPiece.position - 10 >= 0 && checkedPiece.getColour(squares[checkedPiece.position - 10]) != Piece::Black && checkedPiece.position / 8 - (checkedPiece.position - 10) / 8 == 1)
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
                        if (checkedPiece.position - 15 >= 0 && checkedPiece.getColour(squares[checkedPiece.position - 15]) != Piece::Black && checkedPiece.position / 8 - (checkedPiece.position - 15) / 8 == 2)
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
                        if (checkedPiece.position - 17 >= 0 && checkedPiece.getColour(squares[checkedPiece.position - 17]) != Piece::Black && checkedPiece.position / 8 - (checkedPiece.position - 17) / 8 == 2)
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
                        if (!checkIfCheck || (whiteLong && checkedPiece.position == 60 && squares[57] + squares[58] + squares[59] == 0 && IsNotTakeable(Piece::Black, 57) && IsNotTakeable(Piece::Black, 58) && IsNotTakeable(Piece::Black, 59) && IsNotTakeable(Piece::Black, 60)))
                        {
                            activeMove.casLong = true;
                            activeMove.targetSquare = 58;
                            legalMoves.push_back(activeMove);
                            activeMove.casLong = false;
                        }
                        if (!checkIfCheck || (whiteShort && squares[61] + squares[62] == 0 && IsNotTakeable(Piece::Black, 61) && IsNotTakeable(Piece::Black, 62) && IsNotTakeable(Piece::Black, 60) && checkedPiece.position == 60))
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
                        if (!checkIfCheck || (blackLong && squares[1] + squares[2] + squares[3] == 0 && IsNotTakeable(Piece::White, 1) && IsNotTakeable(Piece::White, 2) && IsNotTakeable(Piece::White, 3) && IsNotTakeable(Piece::White, 4) && checkedPiece.position == 4))
                        {
                            activeMove.casLong = true;
                            activeMove.targetSquare = 2;
                            legalMoves.push_back(activeMove);
                            activeMove.casLong = false;
                        }
                        if (!checkIfCheck || (blackShort && squares[5] + squares[6] == 0 && IsNotTakeable(Piece::White, 5) && IsNotTakeable(Piece::White, 6) && IsNotTakeable(Piece::White, 4) && checkedPiece.position == 4))
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
    //None = 0, King = 5, Pawn = 6, Knight = 2, Bishop = 3, Rook = 1, Queen = 4, White = 8, Black = 16;
    inline static int pawnValue = 100;
    inline static int knightValue = 320;
    inline static int bishopValue = 330;
    inline static int rookValue = 500;
    inline static int queenValue = 900;
    inline static int kingValue = 20000;

    inline static int pawnTable[64]{
        0,  0,  0,  0,  0,  0,  0,  0,
        50, 50, 50, 50, 50, 50, 50, 50,
        10, 10, 20, 30, 30, 20, 10, 10,
        5,  5, 10, 25, 25, 10,  5,  5,
        0,  0,  0, 30, 30,  0,  0,  0,
        5, -5,-10,  0,  0,-10, -5,  5,
        5, 10, 10,-100,-100, 10, 10,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    inline static int knightTable[64]{
        -50,-40,-30,-30,-30,-30,-40,-50,
        -40,-20,  0,  0,  0,  0,-20,-40,
        -30,  0, 10, 15, 15, 10,  0,-30,
        -30,  5, 15, 20, 20, 15,  5,-30,
        -30,  0, 15, 20, 20, 15,  0,-30,
        -30,  5, 10, 15, 15, 10,  5,-30,
        -40,-20,  0,  5,  5,  0,-20,-40,
        -50,-40,-30,-30,-30,-30,-40,-50,
    };

    inline static int bishopTable[64]{
        -20,-10,-10,-10,-10,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5, 10, 10,  5,  0,-10,
        -10,  5,  5, 10, 10,  5,  5,-10,
        -10,  0, 10, 10, 10, 10,  0,-10,
        -10, 10, 10, 10, 10, 10, 10,-10,
        -10,  5,  0,  0,  0,  0,  5,-10,
        -20,-10,-10,-10,-10,-10,-10,-20
    };

    inline static int rookTable[64]{
        0,  0,  0,  0,  0,  0,  0,  0,
        5, 10, 10, 10, 10, 10, 10,  5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        -5,  0,  0,  0,  0,  0,  0, -5,
        0,  0,  0,  5,  5,  0,  0,  0
    };

    inline static int queenTable[64]{
        -20,-10,-10, -5, -5,-10,-10,-20,
        -10,  0,  0,  0,  0,  0,  0,-10,
        -10,  0,  5,  5,  5,  5,  0,-10,
        -5,  0,  5,  5,  5,  5,  0, -5,
        0,  0,  5,  5,  5,  5,  0, -5,
        -10,  5,  5,  5,  5,  5,  0,-10,
        -10,  0,  5,  0,  0,  0,  0,-10,
        -20,-10,-10, -5, -5,-10,-10,-20
    };

    inline static int kingTable[64]{
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -30,-40,-40,-50,-50,-40,-40,-30,
        -20,-30,-30,-40,-40,-30,-30,-20,
        -10,-20,-20,-20,-20,-20,-20,-10,
        20, 20,  0,  0,  0,  0, 20, 20,
        20, 30, 10,  0,  0, 10, 30, 20
    };


public:
    inline static int maxDepth = 3;

    static float Eval(Board board, int playerToMove)
    {
        float eval = 0;
        if (playerToMove == Piece::White)
        {
            Board testingBoard{ board.squares, board.blackPieces, board.whitePieces, board.blackLong, board.blackShort, board.whiteLong, board.whiteShort };
            testingBoard.legalMoves.clear();
            for (int piece : board.whitePieces)
            {
                cout << "white piece: " << piece << endl;
                testingBoard.CheckLegalMoves(piece, true);
            }
            if (testingBoard.legalMoves.empty())
            {
                if (testingBoard.whiteInCheck)
                {
                    return INT_MIN;
                    cout << "security alert, security alert, 0-1" << endl;
                }
                else
                {
                    return 0;
                }
            }
        }
        if (playerToMove == Piece::Black)
        {
            Board testingBoard{ board.squares, board.blackPieces, board.whitePieces, board.blackLong, board.blackShort, board.whiteLong, board.whiteShort };
            testingBoard.legalMoves.clear();
            for (int piece : board.blackPieces)
            {
                testingBoard.CheckLegalMoves(piece, true);
            }
            if (testingBoard.legalMoves.empty())
            {
                if (testingBoard.blackInCheck)
                {
                    return INT_MAX;
                    cout << "security alert, security alert, 1-0" << endl;
                }
                else
                {
                    return 0;
                }
            }
        }

        for (int pos : board.blackPieces)
        {
            int piece = board.squares[pos];
            switch (Piece::getType(piece))
            {
            case Piece::Pawn:
                    eval -= (pawnValue + pawnTable[63 - pos]);
                break;

            case Piece::Knight:
                eval -= (knightValue + knightTable[63 - pos]);
                break;

            case Piece::Bishop:
                eval -= (bishopValue + bishopTable[63 - pos]);
                break;

            case Piece::Rook:
                eval -= (rookValue + rookTable[63 - pos]);
                break;

            case Piece::Queen:
                eval -= (queenValue + queenTable[63 - pos]);
                break;

            case Piece::King:
                eval -= (kingValue + kingTable[63 - pos]);
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
                    eval += (pawnValue + pawnTable[pos]);
                break;

            case Piece::Knight:
                eval += (knightValue + knightTable[pos]);
                break;

            case Piece::Bishop:
                eval += (bishopValue + bishopTable[pos]);
                break;

            case Piece::Rook:
                eval += (rookValue + rookTable[pos]);
                break;

            case Piece::Queen:
                eval += (queenValue + queenTable[pos]);
                break;

            case Piece::King:
                eval += (kingValue + kingTable[pos]);
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
        board.legalMoves.clear();
        cout << "depth: " << depth << endl;
        //reached max search depth, return the evaluation
        if (depth <= 0)
        {
            return Eval(board, whiteToPlay ? Piece::White : Piece::Black);
        }
        
        //not max depth yet, check possible moves
        if (whiteToPlay)
        {
            //max eval starts at the minimum
            float maxEval = INT_MIN;

            for (int piece : board.whitePieces) //using the input board here so the list doesn't change during the loop
            {
                cout << "white piece: " << piece << endl;
                board.CheckLegalMoves(piece, true);
                cout << "fecking tested" << endl;
                for (Move& testedMove : board.legalMoves)
                {
                    Board testingBoard{ board.squares, board.blackPieces, board.whitePieces, board.blackLong, board.blackShort, board.whiteLong, board.whiteShort };
                    testingBoard.PlayMove(testedMove, true);
                    float eval = minimax(depth - 1, false, alpha, beta, testingBoard);
                    if (eval > maxEval)
                    {
                        maxEval = eval;
                    }
                    cout << "eval: " << maxEval << endl;
                    alpha = fmax(alpha, maxEval);
                    if (beta <= alpha)
                    {
                        break;
                    }
                }
                board.legalMoves.clear();
            }
            if (maxEval == INT_MIN) //if no move changed maxEval, either there's forced mate or there are no legal moves
            {
                if (!board.whiteInCheck)
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
            float minEval = INT_MAX;
            
            for (int piece : board.blackPieces) //using the input board here so the list doesn't change during the loop
            {
                //cout << "black piece: " << piece << endl;
                board.CheckLegalMoves(piece, true);
                for (Move& testedMove : board.legalMoves)
                {
                    Board testingBoard{ board.squares, board.blackPieces, board.whitePieces, board.blackLong, board.blackShort, board.whiteLong, board.whiteShort };
                    testingBoard.PlayMove(testedMove, true);
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
                board.legalMoves.clear();
            }
            if (minEval == INT_MAX) //if no move improved minEval, either there's forced mate or there are no legal moves
            {
                if (board.IsNotTakeable(Piece::White, board.FindKing(Piece::Black)))
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
    
    

    int activePlayer = Piece::White;

    board.giveFeedback = true;

    board.texture.loadFromFile("images/board0.png");
    Sprite boardSprite;
    boardSprite.setTexture(board.texture);

    

    Texture pieceTexture;
    pieceTexture.loadFromFile("images/figures.png");
    Sprite pieceSprite;
    pieceSprite.setTexture(pieceTexture);

    board.LoadPosition("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    RenderWindow window{VideoMode(453, 453), "Chess"};

    while (window.isOpen())
    {
        Event event;

        sf::Vector2i pos = Mouse::getPosition(window);

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
                                        activePlayer = Piece::Black;
                                        break;
                                    }
                                    else
                                    {
                                        board.PlayMove(move, true);
                                        activePlayer = Piece::Black;
                                        break;
                                    }                                    
                                }
                            }
                            squareSelected = false;
                            activePiece.position = -1;
                            promotionSquare = -1;
                            Engine::Eval(board, activePlayer);
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

        if (board.FindKing(Piece::White) == -1 || board.FindKing(Piece::Black) == -1)
        {
            cout << "Are you kidding ??? What the **** are you talking about man ? You are a biggest looser i ever seen in my life ! You was doing PIPI in your pampers when i was beating players much more stronger then you! You are not proffesional, because proffesionals knew how to lose and congratulate opponents, you are like a girl crying after i beat you! Be brave, be honest to yourself and stop this trush talkings!!! Everybody know that i am very good blitz player, i can win anyone in the world in single game! And \"w\"esley \"s\"o is nobody for me, just a player who are crying every single time when loosing, ( remember what you say about Firouzja ) !!! Stop playing with my name, i deserve to have a good name during whole my chess carrier, I am Officially inviting you to OTB blitz match with the Prize fund! Both of us will invest 5000$ and winner takes it all! " << endl << "I suggest all other people who's intrested in this situation, just take a look at my results in 2016 and 2017 Blitz World championships, and that should be enough... No need to listen for every crying babe, Tigran Petrosyan is always play Fair ! And if someone will continue Officially talk about me like that, we will meet in Court! God bless with true! True will never die ! Liers will kicked off..." << endl;
            return 0;
        }

        if (activePlayer == Piece::Black && promotionSquare == -1)
        {
            list<Move> bestMoves;
            float minEval = INT_MAX;

            board.legalMoves.clear();
            for (int piece : board.blackPieces)
            {
                board.CheckLegalMoves(piece, true);
                cout << "black piece: " << piece << endl;
            }
            for (Move& move : board.legalMoves)
            {
                Board testingBoard{ board.squares, board.blackPieces, board.whitePieces, board.blackLong, board.blackShort, board.whiteLong, board.whiteShort };
                testingBoard.PlayMove(move, true);
                float eval = Engine::minimax(Engine::maxDepth - 1, true, INT_MIN, INT_MAX, testingBoard);
                if (eval == minEval)
                {
                    bestMoves.push_back(move);
                }
                if (eval < minEval)
                {
                    bestMoves.clear();
                    bestMoves.push_back(move);
                    minEval = eval;
                }
            }
            if (bestMoves.empty())
            {
                cout << "game fucking ended bruv" << endl;
                window.close();
                return 0;
            }
            cout << "eval: " << minEval << endl;
            int moveIndex = rand() % bestMoves.size();
            auto bestMove = bestMoves.begin();
            advance(bestMove, moveIndex);
            board.PlayMove(*bestMove, true);
            activePlayer = Piece::White;
        }
    }
    return 0;
}