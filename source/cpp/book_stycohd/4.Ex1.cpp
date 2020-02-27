int main()
{
	enum SQUARE  
	{
		NOTHING = 0,
		PAWN,
		ROOK,
		KNIGHT,
		BISHOP,
		KING,
		QUEEN
	};

	SQUARE ChessBoard[8][8];

	// Initialize the squares containing rooks
	ChessBoard[0][0] = ChessBoard[0][7] = ROOK;
	ChessBoard[7][0] = ChessBoard[7][7] = ROOK;

	return 0;
}