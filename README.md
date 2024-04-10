GLOBAL BRANCH PREDICTOR:

I have set up a globalHistory to store global historical information. Recorded the history of the most recent branch (whether it was taken or not). Whenever a conditional branch instruction is executed, CondBranch is called. He uses the current global history to predict whether a branch will be taken. Afterwards, the predicted results are compared with the actual branch results to update the statistics of successful/failed predictions, as well as update the global history and prediction table status.

Compared to Bimodal prediction, Global branch prediction can better understand the context of program execution by using global historical information. This method can capture the correlation between different branches, which may not be as obvious in the local history components of dual-mode prediction.
