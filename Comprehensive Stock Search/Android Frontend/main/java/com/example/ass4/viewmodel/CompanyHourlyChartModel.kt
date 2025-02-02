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
import com.example.ass4.api.models.ResponseData

class CompanyHourlyChartModel : ViewModel() {
    private val _hourlyResults = MutableStateFlow<ResponseData?>(null)
    val hourlyResults: StateFlow<ResponseData?> = _hourlyResults

    fun getCompany(companyId: String) {
        viewModelScope.launch {
            try {
                val results = RetrofitInstance.apiService.getCompany(companyId)
                _hourlyResults.value = results
            }
            catch (e: Exception) {
                Log.e("ViewModel", "Error fetching company data", e)
                _hourlyResults.value = null
            }
        }
    }
}