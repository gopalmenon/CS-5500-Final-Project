#include <tbb\blocked_range.h>
#include <tbb\parallel_for.h>

#include "GameBoard.hpp"
#include "GameSlot.hpp"

#include <algorithm> 
#include <cassert>
#include <climits>
#include <iostream>
#include <utility>
#include <vector>

namespace controller {

	class ConnectFourGame {

	private:

		//Constants
		const static bool DEFAULT_FIRST_PLAYER_IS_USER = true;
		const static int DEFAULT_DIFFICULTY_LEVEL = 2;
		const static int HEURISTIC_SCORE_FOR_ONE_IN_ROW = 1;
		const static int HEURISTIC_SCORE_FOR_TWO_IN_ROW = 3;
		const static int HEURISTIC_SCORE_FOR_THREE_IN_ROW = 9;
		const static int HEURISTIC_SCORE_FOR_FOUR_IN_ROW = INT_MAX;
		const static int COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW = 3;

		int gameDifficultyLevel;
		bool firstPlayerIsUser;
		model::GameBoard gameBoard;

		//Check if this is a valid play given the game board dimensions and coins already played
		bool isValidPlay(int dropInColumn) {

			//First check if the column number is valid as per the dimensions of the game board
			if (this->gameBoard.isValidColumn(dropInColumn)) {

				//Next check if there is at least one empty slot in the column. i.e. check if the top slot is empty
				if (this->gameBoard.isEmptyAt(dropInColumn)) {
					return true;
				}
				else {
					return false;
				}

			}
			else {
				return false;
			}

		}

		//Return the index corresponding to the top available position
		int getAvailableSlot(int columnNumber) {

			//Make sure that this is a valid play
			assert(isValidPlay(columnNumber));

			//Find the top and bottom rows in the columns
			int bottomRowInColumn = columnNumber + this->gameBoard.getNumberOfColumns() * (this->gameBoard.getNumberOfRows() - 1);
			int topRowInColumn = columnNumber;

			//Start with bottom row and go one cell above at a time till an empty one is found
			for (int slotCounter = bottomRowInColumn; slotCounter >= topRowInColumn; slotCounter -= this->gameBoard.getNumberOfColumns()) {
				if (this->gameBoard.isEmptyAt(slotCounter)) {
					return slotCounter;
				}
			}

			//Exit the program as if it reaches here as this should never happen
			assert(false);

			return 0;
		}

		//Drop a coin into one of the columns
		void dropCoin(int dropInColumn, bool isUserCoin) {

			//First check if the play is a valid one
			if (this->isValidPlay(dropInColumn)) {

				//Place a user coin in the top available position
				this->gameBoard.getGameSlot(getAvailableSlot(dropInColumn)).putCoin(isUserCoin);

				if (wasWinningPlay(dropInColumn, true)) {
					endTheGame(true);
				}
				else {
					//Make a move to best counter the user move
					int columnToPlay = counterUserMove();
					int rowToPlay = getAvailableSlot(columnToPlay);
					this->gameBoard.getGameSlot(this->gameBoard.getBoardIndex(rowToPlay, columnToPlay)).putCoin(false);
				}
			}
		}

		//Check if the last play was a winning play
		bool wasWinningPlay(int columnPlayed, bool isUserCoin) {

			//Compute hueristic scores for horizontal, vertical and diagonal four coins in a row resulting from 
			//coin being dropped in column
			int horizontalHueristicScore = getHorizontalHueristicScore(columnPlayed, gameBoard, isUserCoin);
			int verticalHueristicScore = getVerticalHueristicScore(columnPlayed, gameBoard, isUserCoin);
			int positiveSlopeHueristicScore = getPositiveSlopeHueristicScore(columnPlayed, gameBoard, isUserCoin);
			int negativeSlopeHueristicScore = getNegativeSlopeHueristicScore(columnPlayed, gameBoard, isUserCoin);

			//If it was a winning move, then return with indicator saying so
			if (horizontalHueristicScore == INT_MAX ||
				verticalHueristicScore == INT_MAX ||
				positiveSlopeHueristicScore == INT_MAX ||
				negativeSlopeHueristicScore == INT_MAX) {

				return true;
			}
			else {
				return false;
			}
		}

		//Show message saying who won and disable game controls
		void endTheGame(bool userWon) {

			//TODO display message saying the user won/lost and disable game playing controls
		}

		//Consider all possible moves and play the one with the best hueristic score that maximizes the chance of winning 
		int counterUserMove() {

			std::vector<int> moveScores(this->gameBoard.getNumberOfColumns());
			int depth = this->gameDifficultyLevel;

			//Find best move by considering all columns in parallel using the Map pattern
			tbb::parallel_for(
				tbb::blocked_range<int>(0, this->gameBoard.getNumberOfColumns() + 1),
				[=, &moveScores](tbb::blocked_range<int> range) {

				for (int columnCounter = range.begin(); columnCounter != range.end(); ++columnCounter) {
					moveScores.at(columnCounter) = getMoveHueristicScore(depth, columnCounter, true, this->gameBoard);
				}
			}
			);

			int bestMove = 0, bestScore = -1 * INT_MAX;
			for (int moveCounter = 0; moveCounter < this->gameBoard.getNumberOfColumns(); ++moveCounter) {
				if (moveScores.at(moveCounter) > bestScore) {
					bestScore = moveScores.at(moveCounter);
					bestMove = moveCounter;
				}
			}

			return bestMove;

		}

		//Compute best heuristic score for opponent move
		int bestHeuristicScoreForOpponentMove(int depth, bool isUserCoin, model::GameBoard gameBoard) {

			std::vector<int> moveScores(gameBoard.getNumberOfColumns());
			//Do a map to find the move with the highest score
			tbb::parallel_for(
				tbb::blocked_range<int>(0, this->gameBoard.getNumberOfColumns() + 1),
				[=, &moveScores](tbb::blocked_range<int> range) {

				for (int columnCounter = range.begin(); columnCounter != range.end(); ++columnCounter) {
					moveScores.at(columnCounter) = getMoveHueristicScore(depth, columnCounter, isUserCoin, gameBoard);
				}
			}
			);

			int bestMove = 0, bestScore = -1 * INT_MAX;
			for (int moveCounter = 0; moveCounter < gameBoard.getNumberOfColumns(); ++moveCounter) {
				if (moveScores.at(moveCounter) > bestScore) {
					bestScore = moveScores.at(moveCounter);
					bestMove = moveCounter;
				}
			}

			return bestMove;

		}

		//Compute and return the hueristic score for the move
		int getMoveHueristicScore(int depth, int columnPlayed, bool isUserCoin, model::GameBoard gameBoard) {

			//If maximum depth has been reached, then return
			if (depth == 0) {
				return 0;
			}
			
			//Compute hueristic scores for horizontal, vertical and diagonal four coins in a row resulting from 
			//coin being dropped in column
			int horizontalHueristicScore = getHorizontalHueristicScore(columnPlayed, gameBoard, isUserCoin);
			int verticalHueristicScore = getVerticalHueristicScore(columnPlayed, gameBoard, isUserCoin);
			int positiveSlopeHueristicScore = getPositiveSlopeHueristicScore(columnPlayed, gameBoard, isUserCoin);
			int negativeSlopeHueristicScore = getNegativeSlopeHueristicScore(columnPlayed, gameBoard, isUserCoin);

			//If it was a winning move, then return with indicator saying so
			if (horizontalHueristicScore == INT_MAX ||
				verticalHueristicScore == INT_MAX ||
				positiveSlopeHueristicScore == INT_MAX ||
				negativeSlopeHueristicScore == INT_MAX) {

				return INT_MAX;
			}

			int heuristicScoreForCurrentMove = horizontalHueristicScore +
				                               verticalHueristicScore +
				                               positiveSlopeHueristicScore +
				                               negativeSlopeHueristicScore;

			//Make a copy of the current gameboard to simulate a dropped coin
			model::GameBoard whatIfGameBoard = model::GameBoard(gameBoard);
			whatIfGameBoard.forceDropCoin(columnPlayed, isUserCoin ? false : true);

			return heuristicScoreForCurrentMove - bestHeuristicScoreForOpponentMove(depth - 1, isUserCoin ? false : true, whatIfGameBoard);
		}

		//Check if the cells between from and to index are of the required type or empty. 
		//Also count the number of cells of the required type.
		bool isEmptyOrRequiredType(int fromIndex, int toIndex, bool userCoinPlayed, int& hueristicScore, model::GameBoard gameBoard) {

			int coinCount = 0, nextRow, nextColumn, nextIndex;
			bool firstTime = true;

			int fromRow = gameBoard.getRowNumber(fromIndex);
			int fromColumn = gameBoard.getColumnNumber(fromIndex);
			int toRow = gameBoard.getRowNumber(toIndex);
			int toColumn = gameBoard.getColumnNumber(toIndex);

			for (int column = 0; column <= COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW; ++column) {

				if (fromRow == toRow) {//horizontal sequence
					nextRow = fromRow;
					nextColumn = fromColumn + column;
					nextIndex = gameBoard.getBoardIndex(nextRow, nextColumn);
				}
				else if (fromColumn == toColumn) {//vertical sequence
					nextRow = fromRow + column;
					nextColumn = fromColumn;
					nextIndex = gameBoard.getBoardIndex(nextRow, nextColumn);
				}
				else if (fromRow > toRow && fromColumn < toColumn) {//diagonal going up
					if (firstTime) {
						firstTime = false;
						nextIndex = fromIndex;
					}
					else {
						nextIndex = gameBoard.getDiagonalCellToRightGoingUp(nextIndex);
					}					

				}
				else if (fromRow < toRow && fromColumn < toColumn) {//diagonal going down
					if (firstTime) {
						firstTime = false;
						nextIndex = fromIndex;
					}
					else {
						nextIndex = gameBoard.getDiagonalCellToRightGoingDown(nextIndex);
					}

				}

				if (gameBoard.getGameSlot(nextIndex).hasComputerCoin()) {
					if (userCoinPlayed) {
						return false;
					}
					else {
						++coinCount;
					}
				}
				else if (gameBoard.getGameSlot(nextIndex).hasUserCoin()) {
					if (userCoinPlayed) {
						++coinCount;
					}
					else {
						return false;
					}
				}

			}

			if (coinCount == 1) {
				hueristicScore = HEURISTIC_SCORE_FOR_ONE_IN_ROW;
			}
			else if (coinCount == 2) {
				hueristicScore = HEURISTIC_SCORE_FOR_TWO_IN_ROW;
			}
			else if (coinCount == 3) {
				hueristicScore = HEURISTIC_SCORE_FOR_THREE_IN_ROW;
			}
			else if (coinCount == 4) {
				hueristicScore = HEURISTIC_SCORE_FOR_FOUR_IN_ROW;
			}
			else {
				hueristicScore = 0;
			}

			return true;
		}

		//Heuristic score for dropping a coin in the column played for all potential four-in-a-row horizontal configurations.
		int getHorizontalHueristicScore(int columnPlayed, model::GameBoard gameBoard, bool isUserCoin) {

			int slidingWindowStartPosition, hueristicScore, totalHueristicScore = 0, endColumn;
			if (columnPlayed >= COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW) {
				slidingWindowStartPosition = columnPlayed - COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
			}
			else {
				slidingWindowStartPosition = 0;
			}

			//Consider each four-in-a-row window starting from three before dropped column and ending at the dropped position
			for (int startColumn = slidingWindowStartPosition; startColumn <= columnPlayed; ++startColumn) {

				if (startColumn + COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW > gameBoard.getNumberOfColumns - 1) {
					break;
				}
				else {
					endColumn = startColumn + COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
				}

				int rowContainingDroppedCoin = gameBoard.getRowNumber(getAvailableSlot(columnPlayed));
				if (isEmptyOrRequiredType(gameBoard.getBoardIndex(rowContainingDroppedCoin, startColumn), gameBoard.getBoardIndex(rowContainingDroppedCoin, endColumn), isUserCoin, hueristicScore, gameBoard)) {
					totalHueristicScore += hueristicScore;
				}
			}

			return totalHueristicScore;
		}

		//Heuristic score for dropping a coin in the column played for all potential four-in-a-row vertical configurations.
		int getVerticalHueristicScore(int columnPlayed, model::GameBoard gameBoard, bool isUserCoin) {

			int slidingWindowStartPosition, hueristicScore, totalHueristicScore = 0, endRow;
			int coinDroppedInRow = gameBoard.getRowNumber(getAvailableSlot(columnPlayed));

			if (coinDroppedInRow >= COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW) {
				slidingWindowStartPosition = coinDroppedInRow - COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
			}
			else {
				slidingWindowStartPosition = 0;
			}

			//Consider each four-in-a-row window starting from three before dropped column and ending at the dropped position
			for (int startRow = slidingWindowStartPosition; startRow <= coinDroppedInRow; ++startRow) {

				if (startRow + COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW > gameBoard.getNumberOfRows - 1) {
					break;
				}
				else {
					endRow = startRow + COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
				}

				if (isEmptyOrRequiredType(gameBoard.getBoardIndex(startRow, columnPlayed), gameBoard.getBoardIndex(endRow, columnPlayed), isUserCoin, hueristicScore, gameBoard)) {
					totalHueristicScore += hueristicScore;
				}
			}

			return totalHueristicScore;
		}

		int getPositiveSlopeHueristicScore(int columnPlayed, model::GameBoard gameBoard, bool isUserCoin) {

			int slidingWindowStartRowPosition, slidingWindowStartColumnPosition, hueristicScore, totalHueristicScore = 0, endRow, endColumn;
			int coinDroppedInRow = gameBoard.getRowNumber(getAvailableSlot(columnPlayed));

			if (gameBoard.getNumberOfRows() - 1 - coinDroppedInRow >= COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW && columnPlayed >= COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW) {
				slidingWindowStartRowPosition = coinDroppedInRow + COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
				slidingWindowStartColumnPosition = columnPlayed - COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
			}
			else {
				int rowDistanceFromBottom = gameBoard.getNumberOfRows - 1 - coinDroppedInRow;
				int columnDistanceFromLeft = columnPlayed;
				
				slidingWindowStartRowPosition = coinDroppedInRow + std::min(rowDistanceFromBottom, columnDistanceFromLeft);
				slidingWindowStartColumnPosition = columnPlayed - std::min(rowDistanceFromBottom, columnDistanceFromLeft);
			}

			//Consider each four-in-a-row window starting from three before dropped column and ending at the dropped position
			for (int startRow = slidingWindowStartRowPosition, int startColumn = slidingWindowStartColumnPosition; startColumn <= columnPlayed; --startRow, ++startColumn) {

				if (startColumn + COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW > gameBoard.getNumberOfColumns - 1 ||
					startRow < COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW) {
					break;
				}
				else {
					endRow = startRow - COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
					endColumn = startColumn + COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
				}

				if (isEmptyOrRequiredType(gameBoard.getBoardIndex(startRow, startColumn), gameBoard.getBoardIndex(endRow, startColumn), isUserCoin, hueristicScore, gameBoard)) {
					totalHueristicScore += hueristicScore;
				}
			}

			return totalHueristicScore;
		}

		int getNegativeSlopeHueristicScore(int columnPlayed, model::GameBoard, bool isUserCoin) {

			int slidingWindowStartRowPosition, slidingWindowStartColumnPosition, hueristicScore, totalHueristicScore = 0, endRow, endColumn;
			int coinDroppedInRow = gameBoard.getRowNumber(getAvailableSlot(columnPlayed));

			if (coinDroppedInRow >= COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW && columnPlayed >= COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW) {
				slidingWindowStartRowPosition = coinDroppedInRow - COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
				slidingWindowStartColumnPosition = columnPlayed - COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
			}
			else {
				int rowDistanceFromTop = coinDroppedInRow;
				int columnDistanceFromLeft = columnPlayed;

				slidingWindowStartRowPosition = coinDroppedInRow - std::min(rowDistanceFromTop, columnDistanceFromLeft);
				slidingWindowStartColumnPosition = columnPlayed - std::min(rowDistanceFromTop, columnDistanceFromLeft);
			}

			//Consider each four-in-a-row window starting from three before dropped column and ending at the dropped position
			for (int startRow = slidingWindowStartRowPosition, int startColumn = slidingWindowStartColumnPosition; startColumn <= columnPlayed; ++startRow, ++startColumn) {

				if (startColumn + COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW > gameBoard.getNumberOfColumns - 1 ||
					startRow + COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW < gameBoard.getNumberOfRows() - 1) {
					break;
				}
				else {
					endRow = startRow + COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
					endColumn = startColumn + COLUMN_OR_ROW_DIFFERENCE_FOR_FOUR_IN_A_ROW;
				}

				if (isEmptyOrRequiredType(gameBoard.getBoardIndex(startRow, startColumn), gameBoard.getBoardIndex(endRow, startColumn), isUserCoin, hueristicScore, gameBoard)) {
					totalHueristicScore += hueristicScore;
				}
			}

			return totalHueristicScore;
		}

	public:

		//Default constructor will set game parameters using default values
		ConnectFourGame() {

			gameBoard = model::GameBoard();
			this->firstPlayerIsUser = DEFAULT_FIRST_PLAYER_IS_USER;
			this->gameDifficultyLevel = DEFAULT_DIFFICULTY_LEVEL;
			//TODO computer to go first depending on user setting
		}

		ConnectFourGame(int numberOfRows, int numberOfColumns) {

			gameBoard = model::GameBoard(numberOfRows, numberOfColumns);
			this->firstPlayerIsUser = DEFAULT_FIRST_PLAYER_IS_USER;
			this->gameDifficultyLevel = DEFAULT_DIFFICULTY_LEVEL;
			//TODO computer to go first depending on user setting
		}

		//First player will be determined by user selection
		void setWhoPlaysFirst(bool firstPlayerIsUser) {
			this->firstPlayerIsUser = firstPlayerIsUser;
		}

		//Difficulty level will be set by user
		void setgameDifficultyLevel(int gameDifficultyLevel) {
			this->gameDifficultyLevel = gameDifficultyLevel;
		}

		//User method to drop a coin into one of the columns
		void dropCoin(int dropInColumn) {

			dropCoin(dropInColumn, true);
		}

	};

}