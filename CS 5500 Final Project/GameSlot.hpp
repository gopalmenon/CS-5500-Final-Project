#include <exception>
#include <stdexcept>

//This class represents a slot in the connect four game that can either be empty, have a user coin or a system coin
class GameSlot {

private:

	enum class SlotStates {empty, hasUserCoin, hasComputerCoin};

	SlotStates slotState;

public:

	//Constructor will create an empty slot
	GameSlot() {
		this->slotState = SlotStates::empty;
	}

	//Put a coin into the slot. This is allowed only if the slot is empty
	void putCoin(bool isUserCoin) {

		//If the slot is not empty then this is an unexpected error
		if (this->slotState != SlotStates::empty) {
			throw std::logic_error("The slot is not empty");
		}

		//Put the coin into the slot
		if (isUserCoin) {
			this->slotState = SlotStates::hasUserCoin;
		}
		else {
			this->slotState = SlotStates::hasComputerCoin;
		}
	}

	//Check if slot is empty
	bool isEmpty() {

		if (this->slotState == SlotStates::empty) {
			return true;
		}
		else {
			return false;
		}
	}

	bool hasUserCoin() {
		return this->slotState == SlotStates::hasUserCoin;
	}

	bool hasComputerCoin() {
		return this->slotState == SlotStates::hasComputerCoin;
	}

};