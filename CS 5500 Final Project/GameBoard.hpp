#include <sstream>
#include <vector>

#include "GameSlot.hpp"

namespace model {

	class GameBoard {

	private:

		//Members
		std::vector<GameSlot> gameBoard;
		int numberOfRows, numberOfColumns;
		bool forceDropAllowed;

		//Constants for default values
		const static int DEFAULT_NUMBER_OF_ROWS = 6;
		const static int DEFAULT_NUMBER_OF_COLUMNS = 7;

	public:

		//Default constructor
		GameBoard() {

			this->gameBoard = std::vector<GameSlot>(DEFAULT_NUMBER_OF_ROWS * DEFAULT_NUMBER_OF_COLUMNS, GameSlot::GameSlot());
			this->numberOfRows = DEFAULT_NUMBER_OF_ROWS;
			this->numberOfColumns = DEFAULT_NUMBER_OF_COLUMNS;
			this->forceDropAllowed = false;

		}

		//Constructor with required number of rows and columns
		GameBoard(int numberOfRows, int numberOfColumns) {

			this->gameBoard = std::vector<GameSlot>(numberOfRows * numberOfColumns, GameSlot::GameSlot());
			this->numberOfRows = numberOfRows;
			this->numberOfColumns = numberOfColumns;
			this->forceDropAllowed = false;

		}

		//Constructor with a gameboard as parameter
		GameBoard(std::vector<GameSlot> gameBoard) {
			this->gameBoard = gameBoard;
			this->forceDropAllowed = true;
		}

		int getNumberOfRows() {
			return this->numberOfRows;
		}

		int getNumberOfColumns() {
			return this->numberOfColumns;
		}

		//Check if the column is valid according to the board dimensions
		bool isValidColumn(int columnNumber) {

			if (columnNumber >= 0 && columnNumber < this->numberOfColumns) {
				return true;
			}
			else {
				return false;
			}
		}

		//Return the row number starting with zero
		int getRowNumber(int boardIndex) {

			return boardIndex / this->numberOfColumns;

		}

		//Return the column number starting with zero
		int getColumnNumber(int boardIndex) {

			return boardIndex % this->numberOfColumns;

		}

		//Return index corresponding to row and column numbers passed in as parameters
		int getBoardIndex(int rowNumber, int columnNumber) {
			if (rowNumber <= this->numberOfRows - 1 && columnNumber <= this->numberOfColumns - 1) {
				return rowNumber * this->numberOfColumns + columnNumber;
			}
			else {
				std::stringstream errorMessage;
				errorMessage << "Row " << rowNumber << " and column " << columnNumber << " is not a valid combination.";
				throw std::logic_error(errorMessage.str());
			}
		}

		int getDiagonalCellToRightGoingUp(int boardIndex) {

			int rowNumber = getRowNumber(boardIndex);
			int columnNumber = getColumnNumber(boardIndex);
			if (rowNumber != 0 && columnNumber != this->numberOfColumns - 1) {
				return getBoardIndex(rowNumber - 1, columnNumber + 1);
			}
			else {
				std::stringstream errorMessage;
				errorMessage << "Cell at index " << boardIndex << " is on row " << rowNumber << " and column " << columnNumber << ". Cannot get a diagonal cell going right and up.";
				throw std::logic_error(errorMessage.str());
			}

		}

		int getDiagonalCellToRightGoingDown(int boardIndex) {

			int rowNumber = getRowNumber(boardIndex);
			int columnNumber = getColumnNumber(boardIndex);
			if (rowNumber != this->numberOfRows - 1 && columnNumber != this->numberOfColumns - 1) {
				return getBoardIndex(rowNumber + 1, columnNumber + 1);
			}
			else {
				std::stringstream errorMessage;
				errorMessage << "Cell at index " << boardIndex << " is on row " << rowNumber << " and column " << columnNumber << ". Cannot get a diagonal cell going right and down.";
				throw std::logic_error(errorMessage.str());
			}

		}

		bool isEmptyAt(int boardIndex) {

			if (this->gameBoard.at(boardIndex).isEmpty()) {
				return true;
			}
			else {
				return false;
			}

		}

		GameSlot getGameSlot(int boardIndex) {

			return this->gameBoard.at(boardIndex);

		}

		void forceDropCoin(int columnNumber, bool isUserCoin) {

			if (this->forceDropAllowed) {

				//Find the top and bottom rows in the columns
				int bottomRowInColumn = columnNumber + this->numberOfColumns * (this->numberOfRows - 1);
				int topRowInColumn = columnNumber;

				//Start with bottom row and go one cell above at a time till an empty one is found
				for (int slotCounter = bottomRowInColumn; slotCounter >= topRowInColumn; slotCounter -= this->numberOfColumns) {
					if (this->gameBoard.at(slotCounter).isEmpty()) {
						this->gameBoard.at(slotCounter).putCoin(isUserCoin);
						break;
					}
				}

			}
			else {
				throw std::logic_error("Force drop is not allowed for this game board");
			}


		}

	};

}