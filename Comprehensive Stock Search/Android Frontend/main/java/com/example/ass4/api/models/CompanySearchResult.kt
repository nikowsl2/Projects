package com.example.ass4.api.models

data class CompanySearchResult(
    val count: Int,
    val result: List<CompanyDescription>
)

data class CompanyDescription(
    val description: String,
    val displaySymbol: String,
    val symbol: String,
    val type: String
)
