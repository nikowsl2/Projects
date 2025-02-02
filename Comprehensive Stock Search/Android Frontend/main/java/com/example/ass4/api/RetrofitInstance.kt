package com.example.ass4.api

import retrofit2.Retrofit
import retrofit2.converter.gson.GsonConverterFactory


object RetrofitInstance {
    private const val BASE_URL = "https://wsl571-assignment3node-418610.wl.r.appspot.com/"
    private val retrofit: Retrofit by lazy {                //from ChatGPT
        Retrofit.Builder()
            .baseUrl(BASE_URL)
            .addConverterFactory(GsonConverterFactory.create())
            .build()
    }
    val apiService: BalanceApiService by lazy {
        retrofit.create(BalanceApiService::class.java)
    }
}