#pragma once

enum class GameStatus {
    Init,
    Ongoing,
    Won,
    Lost
};

class GameState {
public:
    GameState();
    GameState(int initial_mines, int initial_fields);

    void OnFieldFlag();
    void OnFieldUnflag();
    void OnFieldDiscover(bool is_mined);
    void OnBoardGenerate();

    int GetRemainingMines();
    GameStatus GetGameStatus();
private:
    GameStatus game_status;
    int remaining_mines;
    int remaining_fields;

    bool HasWon();
    void CheckAndHandleVictory();
};