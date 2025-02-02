package com.example.ass4.viewmodel


import android.util.Log
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import com.example.ass4.api.RetrofitInstance
import com.example.ass4.api.models.CompanyProfile
import com.example.ass4.api.models.CompanySearchResult
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.map
import kotlinx.coroutines.flow.stateIn

class PortfolioCompanyWatchModel : ViewModel() {
    private val _watchResults = MutableStateFlow<Map<String, CompanyProfile?>>(emptyMap())
    fun PortfolioWatchCompany(ticker: String) {
        viewModelScope.launch {
            try {
                val result = RetrofitInstance.apiService.watchCompany(ticker)
                _watchResults.value = _watchResults.value.toMutableMap().apply {
                    put(ticker, result)
                }
            }
            catch (e: Exception) {
                _watchResults.value = _watchResults.value.toMutableMap().apply {
                    put(ticker, null)
                }
            }
        }
    }

    fun getProfileStateFlow(ticker: String): StateFlow<CompanyProfile?> = _watchResults.map { it[ticker] }.stateIn(viewModelScope, SharingStarted.WhileSubscribed(5000), null)
}