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

class CompanyWatchModel : ViewModel() {
    private val _watchResults = MutableStateFlow<CompanyProfile?>(null)
    val watchResults: StateFlow<CompanyProfile?> = _watchResults
    fun watchCompany(query: String) {
        viewModelScope.launch {
            try {
                val results = RetrofitInstance.apiService.watchCompany(query)
                _watchResults.value = results
            } catch (e: Exception) {
                _watchResults.value = null
            }
        }
    }
}