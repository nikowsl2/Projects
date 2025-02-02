package com.example.ass4.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.launch
import com.example.ass4.api.RetrofitInstance
import com.example.ass4.api.models.CompanySearchResult

class CompanyViewModel : ViewModel() {
    private val _searchResults = MutableStateFlow<CompanySearchResult?>(null)
    val searchResults: StateFlow<CompanySearchResult?> = _searchResults

    fun searchCompany(query: String) {
        viewModelScope.launch {
            try {
                val results = RetrofitInstance.apiService.searchCompany(query)
                _searchResults.value = results
            } catch (e: Exception) {
                _searchResults.value = null
            }
        }
    }
}