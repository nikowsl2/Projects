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

class WatchListModel : ViewModel() {
    private val _watchResults = mutableMapOf<String, MutableStateFlow<CompanyProfile?>>()
    fun watchCompany(ticker: String) {
        viewModelScope.launch {
            try {
                val result = RetrofitInstance.apiService.watchCompany(ticker)
                _watchResults.getOrPut(ticker) { MutableStateFlow(null) }.value = result
            } catch (e: Exception) {

                _watchResults.getOrPut(ticker) { MutableStateFlow(null) }.value = null
            }
        }
    }

    fun getProfileStateFlow(ticker: String): StateFlow<CompanyProfile?> {
        return _watchResults.getOrPut(ticker) { MutableStateFlow(null) }
    }
}
