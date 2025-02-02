
package com.example.ass4.viewmodel

import android.util.Log
import androidx.compose.runtime.mutableStateOf
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.ass4.api.RetrofitInstance
import com.example.ass4.api.Ticker
import com.example.ass4.api.TickerRequest
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

class WatchlistViewModel : ViewModel() {
    private val _watchlist = MutableStateFlow<List<Ticker>>(emptyList())
    val watchlist: StateFlow<List<Ticker>> = _watchlist

    private val _isLoading = MutableStateFlow(false)
    val isLoading: StateFlow<Boolean> = _isLoading

    private val _errorMessage = MutableStateFlow("")
    val errorMessage: StateFlow<String> = _errorMessage

    init {
        fetchWatchlist()
    }

    fun addTicker(ticker: String) {
        viewModelScope.launch {
            _isLoading.value = true
            val response = try {
                RetrofitInstance.apiService.insertTicker(TickerRequest(ticker))
            } catch (e: Exception) {
                _errorMessage.value = "Failed to add ticker: ${e.message}"
                _isLoading.value = false
                null
            }

            if (response?.isSuccessful == true) {
                fetchWatchlist()
            } else {
                _errorMessage.value = "Error: ${response?.message()}"
                _isLoading.value = false
            }
        }
    }

    fun removeTicker(ticker: String) {
        viewModelScope.launch {
            _isLoading.value = true
            val response = try {
                RetrofitInstance.apiService.removeTicker(ticker)
            } catch (e: Exception) {
                _errorMessage.value = "Failed to remove ticker: ${e.message}"
                _isLoading.value = false
                null
            }

            if (response?.isSuccessful == true) {
                fetchWatchlist()
                _errorMessage.value = "Ticker removed successfully."
            } else {
                _errorMessage.value = "Error: ${response?.message()}"
                _isLoading.value = false
            }
        }
    }

    fun fetchWatchlist() {
        _isLoading.value = true
        viewModelScope.launch {
            val response = try {
                RetrofitInstance.apiService.getWatchlist()
            }
            catch (e: Exception) {
                _errorMessage.value = "Failed to fetch data: ${e.message}"
                _isLoading.value = false
                null
            }

            if (response?.isSuccessful == true) {
                _watchlist.value = response.body() ?: emptyList()
            }
            else {
                _errorMessage.value = "Error: ${response?.message()}"
            }
            _isLoading.value = false
        }
        Log.d("WatchListModel", _watchlist.value.toString())
    }
}
