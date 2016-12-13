## 1. 本文名詞定義與前言

* ROTP: 
* OTP: 

本次作業實作的項目有 UCB、UCT 以及 Progressive pruning 等。

## 2. 初始版本的 UCB

在這一版本中，我將 UCB 仍大致上劃分與 Monte Carlo Tree Search 相同的四個步驟外，加上了建置初始 root 盤面的前置工作。

### 2-1. Preprocess in UCB

利用了範例程式中 `ML` 與 `MLED` 的概念，取得目前的所有合法步後，將其列為評定的選項之一。然而每一次的迭代都會需要 root 盤面，此處所指的 root 並非遊戲開始的四子盤面，而是在當前回合中，該玩家在進行下一步棋的先行盤面。加以該良範例程式中的 `History(H)` ，藉由依序執行 **OTP** 界面中的 `play` 指令，重現出該回的 root 盤面。

### 2-2. Selection in UCB

在上一小節提及的選項，便是該回合中的合法步，在這些節點中選擇最高的 UCB score 接續下一步驟。其中 UCB 使用下述公式：

```
// 其中的 N 是目前的迭代次數；c 是一常數，主要使用 1.414。
child.wins / child.visits + c * sqrt( log(N) / child.visits )
```

### 2-3. Expansion in UCB

然而在 UCB 中並不需要再衍生分支，於是對 selection 步驟中所選之節點執行 `play` 指令，確立該棋步以加入至盤面之中。

### 2-4. Simulation in UCB

`board.h` 中的 `is_game_over()` 包裝至 OTP 類別中使用。藉此成為模擬迴圈的跳出判斷條件，反覆執行 `genmove` 指令，而亂數模擬採用原先範例程式的亂數方式（C++ 之 `uniform_int_distribution`）。

### 2-5. Propagation in UCB

利用 `board.h` 中的 `get\_score()`，包裝至 OTP 類別中調用。並在各選項中更新其 wins 與 visits 的數值，其中 wins 的部份為，勝利 +1；和局 +0.5；失敗 +0。

### 2-6. 選擇最高勝率者

在 **2-2** 小節中，以 **wins / visits** 最高者作為本步驟的最佳解。

## 3. 基於樹狀結構的 UCT


### 3-1.



