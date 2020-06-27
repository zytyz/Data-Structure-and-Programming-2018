# Data Structure and Programming-2018 Fall

## Data Structure Design
1. 每個gate都是class CirGate的一個object，裡面存gateID, gateType, simValue還有在sat會用到的variable。
2. 每種gate都是CirGate 的繼承，總共有CirAigGate, CirPIGate, CirPOGate, CirConst, CirUnderGate。
3. 會依照每一種gate去存fanInList 或是 fanOutList。這兩個list是一個Fan的vector。
4. Fan 是自己定義的一個class，理論儲存一個CirGate*和一個bool 數值，主要是把fanin/ fanout的gate的pointer和這個fanin/ fanout要不要invert包裝起來。
5. CirMgr裡面有以下幾樣東西：
	1.	一個map，儲存所有的gate的pointer和gateID，型態是map<unsigned, CirGate*>。
	2.	三個GateList（CirGate* 的vector），包含PIList, POList, DfsList。DfsList是每次要用之前會先從所有的PO Depth first search完更新的。
	3.	一個hash，型態是unordered_map<size_t, CirGate*>，在strash的時候會用到。
	4.	一個FECGroups，型態是vector<GateList*> 是一個儲存所有目前的FECGroups的vector。
6. 以上架構是自己寫的沒有參考別人的。

## Algorithms
### Simulation：
1. 對於每個PI產生或是透過讀檔得到一組pattern，把他包好存成一個vector<size_t> input。
2. 呼叫CirMgr::simulationOnce，會利用剛才的input simulation一次，主要包含以下的動作：
	* 先透過從PO去recursive (post order) 呼叫每個gate的CirGate::simulationGate，把每個gate的simvalue更新。
		* CirAigGate 就是把他的fanin的simValue bitwise and，但要考慮inverting。
		* CirPIGate 的simvalue是input裡面的。
		* CirPOGate的simvalue是看他唯一的fanin的simvalue決定。
		* CirConstGate的simvalue是0。
	* 全部simulate完後就要收集這次剩下的FECGroups
		* 第一次simualtion完要把所有的dfslist裡的aig gate加到一個fecGrp存在FECGoups，但不管dfs會不會碰到const 0 gate都應該要加它。
		* 把現有的FECGroup一個一個看，在每個group裡面開一個local hash (unordered_map<size_t,CirGate*> 裡面存的是simvalue對應到CirGate*的關係，如果hash裡面已經有一個相同或是完全相反的simvalue的gate，代表他們是fec pair，所以再開一個fecGrp把他們存著。
		* 為了避免一個gate已經確定有fecGrp可是搜尋他的fecGrp太久（會搜尋的清況是因為可能這個fecGrp有三個以上的gate，前兩個gate就會創新的fecGrp但後面的gate就要搜尋這個fecGrp而不是在創新的），所以有開一個loacl hash去儲存simValue對應到的新的fecGrp的address，這樣搜尋起來比較快。
	* 收集完FECGroups之後，要對每個group排序。這裡是利用#include<sort>的方法，去對每個CirGate*進行排序，因為是自己定義的object，所以有寫一個 CirGate* 的比較大小的sorting function，透過gate的gateID比較大小。
3.	如果是cirsim -r的話，就要重複上面的simulationOnce多次直到FECGroups的大小不再因為simulationOnce而改變，有設定一個threshold，當連續失敗的次數超過threshold就停止。Threshold的數值是 (log(PIList.size()))+5，因為覺得如果PIList的大小越大，就應該要多simualtion幾次。

### Strash
1.	CirMgr裡面有個hash，針對每個gate產生的key和CirGate*組成一個pair會insert到這個hash裡面，如果裡面已經有一樣的key了，就來比較是不是連inverting input的方式都一樣，是的話就把這兩個gate merge在一起。
2.	每個gate產生的key和兩個fanin的pointer有關，主要是把兩個pointer p, q湊成一個新的size_t key = (p>>3)+(q>>6)。要注意要把比較大的pointer當成p，小的當成q，這樣兩個fanin一樣但順序不同的gate才會有相同的key。
3.	Strash的方式也是從PO post order strash gate，這樣才可以確保是從PI strash到PO。
4.	hashmap是用open source(#include <unordered_map>)。
 
### Fraig
1.	對於_FECGroups的每一個fecGrp都去檢查是否是sat（呼叫CirMgr的satOneFecGrp(_FECGroups[i])）
2.	satOneFecGrp(_FECGroups[i])的內容
	* 對於每一個fecGrp，先把裡面的gate排序好，排序的方式是根據dfsList中的順序，因為如果在dfs的順序越前面，代表證明起來會比較快。但如果碰到const 0而且const0不在dfsList裡面的話，就把const 0擺在最前面。
	* 開一個local 的vector<GateList*> allPiles，收集這個fecGrp在利用sat證明完之後會變成幾堆，每堆(piles)是一個GateList*。
	* 先把fecGrp中的第一個gate存到allPiles的第一個pile，之後的每一個gate都去和allPiles的每一個pile的第一個gate去呼叫satsolver證明。這樣的話如果gateA == gateB，下一個gateC就不用和兩個都證明，只要選第一個證明就好。如果之後的gate證明和某一個pile的第一個相同，那把gate也丟到那個pile裡面。如果某個gate和所有的piles都不相同，就再開一個新的pile，這個pile要存到allPiles，並且存著sat solver給的simPattern。
	* 將所有的gate分好到它的pile後，就可以把整個pile裡的gate都merger在一起了。Merge的方法是留下第一個gate，後面全部合併成第一個gate。在merger以前會再重新把這個pile排序一次（一樣利用dfs裡的順序，因為希望保留的是dfs較前面的gate）
	* 如果這個fecGrp在證明完所有的gate之後，allPiles裡面的pile數量超過一個，代表這個fecGrp其實應該留下每一個pile的第一個gate（其實merge完就只剩一個），這樣之後就可以拿在證明sat時得到的pattern去simulate，就可以把從fecGrp他們分離。
3.	完成以上步驟後，利用剛才得到的所有simPattern重新simulate，就可以消掉所有的fecGrp。

### Optimize
* 利用dfs，由PO去post order進行optimize。

### Sweep
* 對於gateMap（CirMgr裡面）的每一個gate，去檢查他在不在dfslist裡面，不在的話就把這個gate清掉。

## 和ref program比較速度：
1. Simulation：
	* 我的速度大概是ref program的兩倍左右，時間複雜度應該還是linear time，只是在處理collectFECGroups的動作的時候會有比ref program慢一點。
2. Strash, Optimize, sweep的速度都和ref program差不多。
3. Fraig：fraig的速度比ref program快了不少，例如測資sim07.aag，ref program大概要6秒多，我自己的版本只要1秒左右。（我自己的在執行fraig的時候不會去用到strash，順序也和ref program的不同。）

