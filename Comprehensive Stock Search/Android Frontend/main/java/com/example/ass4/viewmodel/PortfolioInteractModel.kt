
package com.example.ass4.viewmodel

import android.util.Log
import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.ass4.api.AggregatedPortfolioItem
import com.example.ass4.api.BalanceUpdateRequest
import com.example.ass4.api.PortfolioItem
import com.example.ass4.api.RetrofitInstance
import com.example.ass4.api.PurchaseRequest
import com.example.ass4.api.SellRequest
import com.example.ass4.api.TickerItem
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

class PortfolioInteractModel : ViewModel() {
    private val _tickers = MutableLiveData<List<TickerItem>>()
    val tickers: LiveData<List<TickerItem>> = _tickers

    private val _portfolioDetails = MutableLiveData<List<PortfolioItem>>()
    val portfolioDetails: LiveData<List<PortfolioItem>> = _portfolioDetails

    private val _aggregatedPortfolioDetails = MutableLiveData<List<AggregatedPortfolioItem>>()
    val aggregatedPortfolioDetails: LiveData<List<AggregatedPortfolioItem>> = _aggregatedPortfolioDetails

    private val _totalPortfolioValue = MutableLiveData<Double>(0.0)
    val totalPortfolioValue: LiveData<Double> = _totalPortfolioValue

    private val _cashBalance = MutableStateFlow<Double?>(null)
    val cashBalance: StateFlow<Double?> = _cashBalance

    private val _refreshTrigger = MutableLiveData<Boolean>(false)
    val refreshTrigger: LiveData<Boolean> = _refreshTrigger

    fun triggerRefresh() {
        _refreshTrigger.value = !(_refreshTrigger.value ?: false)
    }

    val updateResult = MutableLiveData<String>()

    fun updateBalance(newBalance: Double) {
        viewModelScope.launch {
            try {
                val response = RetrofitInstance.apiService.updateBalance(BalanceUpdateRequest(newBalance))
                if (response.isSuccessful) {
                    updateResult.postValue("Update Successful: ${response.body()}")
                } else {
                    updateResult.postValue("Update Failed: ${response.errorBody()?.string()}")
                }
            } catch (e: Exception) {
                updateResult.postValue("Error: ${e.localizedMessage}")
            }
        }
    }

    init {
        fetchCashBalance()
    }

    private fun fetchCashBalance() {
        viewModelScope.launch {
            try {
                val response = RetrofitInstance.apiService.getBalance()
                if (response.isSuccessful && response.body() != null) {
                    _cashBalance.value = response.body()!!.balance
                } else {
                    _cashBalance.value = null
                }
            } catch (e: Exception) {
                _cashBalance.value = null
            }
        }
    }

    fun updateTotalPortfolioValue(addValue: Double) {
        _totalPortfolioValue.value = (_totalPortfolioValue.value ?: 0.0) + addValue
    }

    fun fetchTickers() {
        viewModelScope.launch {
            try {
                val response = RetrofitInstance.apiService.getPortfolioList()
                if (response.isSuccessful && response.body() != null) {
                    _tickers.postValue(response.body())
                    Log.d("ViewModel", "Tickers fetched: ${response.body()?.joinToString { it.ticker }}")
                    fetchPortfolioDetailsForAllTickers(response.body()!!)
                } else {
                    _tickers.postValue(emptyList())
                    Log.d("ViewModel", "Fetching tickers failed: Empty or unsuccessful response")
                }
            } catch (e: Exception) {
                _tickers.postValue(emptyList())
                Log.e("ViewModel", "Error fetching tickers: ${e.message}", e)
            }
        }
    }

    private fun fetchPortfolioDetailsForAllTickers(tickers: List<TickerItem>) {
        viewModelScope.launch {
            val allAggregatedDetails = mutableListOf<AggregatedPortfolioItem>()
            tickers.forEach { ticker ->
                try {
                    val response = RetrofitInstance.apiService.getPortfolioDetails(ticker.ticker)
                    if (response.isSuccessful && response.body() != null) {
                        val portfolioItems = response.body()!!
                        val totalShares = portfolioItems.sumOf { it.shares }
                        val totalCost = portfolioItems.sumByDouble { it.shares * it.purchasePrice }
                        val averageCost = if (totalShares > 0) totalCost / totalShares else 0.0

                        allAggregatedDetails.add(
                            AggregatedPortfolioItem(
                                ticker = ticker.ticker,
                                totalShares = totalShares,
                                averageCost = averageCost,
                                totalCost = totalCost
                            )
                        )
                    }
                } catch (e: Exception) {
                    Log.e("ViewModel", "Error fetching or aggregating data for ticker ${ticker.ticker}: ${e.message}", e)
                }
            }
            _aggregatedPortfolioDetails.postValue(allAggregatedDetails)
        }
    }

    fun purchaseShares(companyTicker: String, companyName: String, numberOfShares: Int, sharePrice: Double): LiveData<String> {
        val result = MutableLiveData<String>()
        viewModelScope.launch {
            val request = PurchaseRequest(companyTicker, companyName, numberOfShares, sharePrice)
            try {
                val response = RetrofitInstance.apiService.purchaseShares(request)
                if (response.isSuccessful) {
                    result.value = "Purchase successful: ${response.body()?.message}"
                    fetchCashBalance()
                } else {
                    result.value = "Purchase failed: ${response.errorBody()?.string()}"
                }
            } catch (e: Exception) {
                result.value = "Error: ${e.localizedMessage}"
            }
        }
        return result
    }

    fun sellShares(ticker: String, sellPrice: Double, numberOfSharesToSell: Int): LiveData<String> {
        val result = MutableLiveData<String>()
        viewModelScope.launch {
            val request = SellRequest(ticker, sellPrice, numberOfSharesToSell)
            try {
                val response = RetrofitInstance.apiService.sellShares(request)
                if (response.isSuccessful) {
                    result.value = "Sale successful: ${response.body()?.message}"
                    fetchCashBalance()
                } else {
                    result.value = "Sale failed: ${response.errorBody()?.string()}"
                }
            } catch (e: Exception) {
                result.value = "Error: ${e.localizedMessage}"
            }
        }
        return result
    }
}


