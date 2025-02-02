
package com.example.ass4.viewmodel

import androidx.lifecycle.LiveData
import androidx.lifecycle.MutableLiveData
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import com.example.ass4.api.RetrofitInstance
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch

class PortfolioViewModel : ViewModel() {
    private val _cashBalance = MutableStateFlow<Double?>(null)
    val cashBalance: StateFlow<Double?> = _cashBalance

    private val _isLoading = MutableLiveData<Boolean>(true)
    val isLoading: LiveData<Boolean> = _isLoading

    init {
        loadData()
        fetchCashBalance()
    }

    private fun loadData() {
        viewModelScope.launch {
            delay(2000)
            _isLoading.value = false
        }
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
}
